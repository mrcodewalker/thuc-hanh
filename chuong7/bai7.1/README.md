# Bài 7.1 - Hệ thống File tùy biến bằng Kernel Module (Custom Filesystem LKM)

## Mục tiêu bài học
Xây dựng một **Driver nhân Linux (Linux Kernel Module - LKM)** tích hợp đồng thời hai thành phần cốt lõi:
1. **Một Character Device (dưới dạng Misc Device)** tại `/dev/myfs_dev`.
2. **Một hệ thống file ảo (Virtual Filesystem - VFS)** có tên là `myfs`.

Hệ thống cho phép gắn kết (**mount**) file thiết bị `/dev/myfs_dev` vào một thư mục bất kỳ. Sau khi gắn kết, người dùng ở không gian ứng dụng (User Space) có thể duyệt thư mục gắn kết để xem các tệp tin chứa trong đó. Đồng thời, tệp tin ảo `device_data.txt` bên trong thư mục này được liên kết đồng bộ bộ nhớ với file thiết bị `/dev/myfs_dev` thông qua vùng đệm dùng chung của nhân hệ điều hành.

---

## 1. Cơ sở lý thuyết về Hệ thống file ảo (VFS) trong Linux

Trong Linux, mọi thứ đều là file (**"Everything is a file"**). Để quản lý các hệ thống file vật lý và ảo khác nhau (như ext4, FAT, procfs, sysfs) một cách đồng nhất, nhân Linux sử dụng lớp giao tiếp **Virtual Filesystem (VFS)**.

### Bốn thực thể chính trong mô hình đối tượng VFS:
1. **Superblock (`struct super_block`):** Đại diện cho một hệ thống file được mount cụ thể. Lưu trữ thông tin metadata của hệ thống file (kích thước block, magic number, danh sách thao tác superblock).
2. **Inode (`struct inode`):** Đại diện cho một đối tượng file vật lý hoặc ảo cụ thể trong bộ nhớ (không lưu tên file). Chứa siêu dữ liệu của file như: quyền truy cập (permission), chủ sở hữu (owner), kích thước (size), và các con trỏ hàm thao tác tương ứng (`struct file_operations`).
3. **Dentry (`struct dentry`):** Đại diện cho một thành phần của đường dẫn (directory entry). Liên kết inode với tên của file để tạo nên cấu trúc cây thư mục.
4. **File (`struct file`):** Đại diện cho một file đang được tiến hành mở bởi tiến trình trong không gian người dùng. Chứa con trỏ đọc/ghi hiện tại (`f_pos`), cờ mở tệp, và dentry liên kết.

---

## 2. Thiết kế Kiến trúc Module (Module Design)

Để kết nối giữa File Thiết bị và Hệ thống file ảo, module được thiết kế dựa trên một vùng nhớ đệm dùng chung duy nhất trong Kernel Space:

```mermaid
graph TD
    subgraph User Space (Không gian người dùng)
        app1[Lệnh echo / cat] -->|Đọc / Ghi trực tiếp| dev_node["File thiết bị: /dev/myfs_dev"]
        app2[Lệnh ls / cat / echo] -->|Duyệt thư mục & Đọc/Ghi file| fs_node["Thư mục mount: /mnt/myfs_mount/device_data.txt"]
    end

    subgraph Kernel Space (Không gian nhân Linux)
        dev_node -->|dev_fops| dev_driver["Misc Device Driver"]
        fs_node -->|myfs_file_operations| vfs_layer["VFS Layer (myfs)"]
        
        dev_driver <-->|Sử dụng chung bộ nhớ| shared_buf[("Bộ đệm Kernel: device_buffer (1024 Bytes)")]
        vfs_layer <-->|Sử dụng chung bộ nhớ| shared_buf
    end
end
```

### Cách thức hoạt động & Đồng bộ:
- **Bộ đệm dùng chung:** Một mảng tĩnh `device_buffer` kích thước `1024 bytes` cùng biến lưu kích thước `device_data_size` và biến loại trừ tương hỗ `myfs_lock` (Mutex) được chia sẻ giữa driver thiết bị và driver hệ thống file.
- **Tại File thiết bị `/dev/myfs_dev`:**
  - Lệnh ghi (`write`) sẽ cập nhật chuỗi dữ liệu mới vào `device_buffer`.
  - Lệnh đọc (`read`) sẽ lấy dữ liệu hiện tại trong `device_buffer` trả về User Space.
- **Tại Hệ thống file `myfs`:**
  - Khi mount, superblock được thiết lập và thư mục gốc (root directory, `inode 100`) được khởi tạo.
  - Thư mục gốc chứa hai tệp ảo:
    - **`readme.txt` (inode 101):** File chỉ đọc chứa hướng dẫn sử dụng.
    - **`device_data.txt` (inode 102):** File đọc/ghi được ánh xạ trực tiếp đến `device_buffer`.
  - Khi ghi hoặc đọc vào `device_data.txt`, nhân Linux gọi hàm `.read`/`.write` của `myfs`, hàm này truy cập trực tiếp vào `device_buffer` toàn cục. Nhờ đó, dữ liệu luôn được đồng bộ ngay lập tức mà không cần đồng bộ qua đĩa vật lý hay các tiến trình trung gian.

---

## 3. Các cấu trúc dữ liệu và hàm quan trọng trong mã nguồn

### Quản lý Hệ thống File:
- `struct file_system_type myfs_type`: Định nghĩa kiểu hệ thống file với tên gọi `"myfs"`, cùng các con trỏ hàm khởi dựng superblock `.mount` và giải phóng `.kill_sb`.
- `myfs_mount(...)`: Gọi helper `mount_nodev` để khởi dựng superblock dạng ảo (không yêu cầu block device thực tế).
- `myfs_fill_super(...)`: Thiết lập tham số superblock và gán inode gốc cho thư mục mount.
- `myfs_get_inode(...)`: Tạo mới một inode và thiết lập các thông số (loại inode, phân quyền, thời gian, operations liên kết).
- `myfs_lookup(...)`: Trả về inode tương ứng cho tệp `readme.txt` (inode 101) hoặc `device_data.txt` (inode 102) khi người dùng truy cập.
- `myfs_readdir(...)`: Hàm cung cấp danh sách tệp ảo (`readme.txt`, `device_data.txt`) khi thực thi lệnh `ls` trên thư mục mount thông qua hàm `dir_emit`.

### Quản lý Thiết bị Misc:
- `struct miscdevice myfs_misc_device`: Đăng ký thiết bị phụ dạng misc với tên `"myfs_dev"`. Hệ thống sẽ tự động sinh file `/dev/myfs_dev`.
- `struct file_operations dev_fops`: Định nghĩa các hàm thao tác trên file thiết bị (`dev_read`, `dev_write`).

---

## 4. Hướng dẫn Biên dịch, Cài đặt và Chạy thử nghiệm

### Yêu cầu hệ thống:
Hệ điều hành Linux hoặc môi trường phát triển nhân Linux có cài đặt trình biên dịch GCC và mã nguồn Kernel headers:
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

### Chạy kiểm thử tự động bằng Script:
Chúng tôi đã xây dựng sẵn tệp lệnh [restart.bash](file:///c:/Users/ADMIN/Thuc-hanh/thuc-hanh/chuong7/bai7.1/restart.bash) giúp tự động thực hiện toàn bộ quy trình:
1. Dọn dẹp module cũ và gỡ mount nếu còn sót.
2. Biên dịch module `myfs.ko`.
3. Nạp module vào nhân bằng `sudo insmod`.
4. Phân quyền và tạo thư mục mount tại `/mnt/myfs_mount`.
5. Mount `/dev/myfs_dev` vào `/mnt/myfs_mount`.
6. Thực hiện liệt kê thư mục, đọc các file ảo.
7. Kiểm thử ghi vào thiết bị -> đọc ở file, và ghi vào file -> đọc ở thiết bị.
8. Unmount và gỡ module, làm sạch bộ nhớ.

Để chạy, thực thi lệnh sau:
```bash
bash restart.bash
```

---

### Các bước thực hiện thủ công bằng lệnh (Để hiểu rõ bản chất):

1. **Biên dịch mã nguồn:**
   ```bash
   make
   ```
   *Tạo ra file module `myfs.ko`.*

2. **Nạp module vào nhân:**
   ```bash
   sudo insmod myfs.ko
   ```

3. **Cấp quyền truy cập cho file thiết bị tự sinh:**
   ```bash
   sudo chmod 666 /dev/myfs_dev
   ```

4. **Tạo thư mục làm điểm gắn kết và mount:**
   ```bash
   sudo mkdir -p /mnt/myfs_mount
   sudo mount -t myfs /dev/myfs_dev /mnt/myfs_mount
   ```

5. **Kiểm tra thư mục mount:**
   ```bash
   ls -la /mnt/myfs_mount
   ```
   *Bạn sẽ thấy `readme.txt` và `device_data.txt`.*

6. **Đọc nội dung và ghi dữ liệu:**
   - Xem hướng dẫn: `cat /mnt/myfs_mount/readme.txt`
   - Đọc dữ liệu chia sẻ ban đầu: `cat /mnt/myfs_mount/device_data.txt`
   - Ghi dữ liệu vào file thiết bị: `echo "Hello Linux Kernel!" > /dev/myfs_dev`
   - Đọc lại từ file trên VFS: `cat /mnt/myfs_mount/device_data.txt`
   - Ghi dữ liệu vào file trên VFS: `echo "Update from VFS file" > /mnt/myfs_mount/device_data.txt`
   - Đọc lại từ file thiết bị: `cat /dev/myfs_dev`

7. **Tháo gỡ hệ thống sau khi hoàn tất:**
   ```bash
   sudo umount /mnt/myfs_mount
   sudo rmmod myfs
   make clean
   ```
