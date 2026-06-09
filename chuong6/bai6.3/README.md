# Bài 6.3 - Trình điều khiển Nhân Linux loại Misc Device

## Mục tiêu

Xây dựng một **Driver nhân Linux (Linux Kernel Module - LKM)** loại **Misc Device** có tên là `misc-module`. 
- Khi nạp Module thành công, hệ thống tự động tạo tệp thiết bị đặc biệt tại **`/dev/misc-module`**.
- Driver hỗ trợ các thao tác cơ bản: Open, Release, Read, Write, và Llseek.
- Viết ứng dụng C ở không gian người dùng (**User Space**) mở tệp `/dev/misc-module` để thực hiện việc ghi nhận thông tin và đọc kiểm chứng hoạt động.

---

## 1. Lý thuyết về Driver nhân Linux và Thiết bị Misc

### Kernel Space vs User Space
Hệ điều hành Linux phân tách bộ nhớ thành hai vùng bảo mật nghiêm ngặt:
- **User Space (Ring 3):** Nơi các ứng dụng thông thường chạy (ví dụ: `test-app`). Không thể truy cập trực tiếp phần cứng hoặc không gian bộ nhớ của hệ thống.
- **Kernel Space (Ring 0):** Nơi nhân Linux và các Drivers hoạt động với đặc quyền cao nhất. 

Để chuyển đổi dữ liệu an toàn qua lại giữa hai không gian này, LKM bắt buộc phải sử dụng các API đặc biệt:
- **`copy_to_user()`**: Sao chép dữ liệu từ vùng nhớ Kernel gửi về vùng nhớ của ứng dụng User Space.
- **`copy_from_user()`**: Sao chép dữ liệu từ bộ đệm của ứng dụng User Space ghi vào bộ đệm của Kernel.

### Misc Device (Thiết bị hỗn hợp) là gì?
Trong Linux, các thiết bị Character Device thông thường yêu cầu lập trình viên phải đăng ký một mã số chính (Major number) và mã số phụ (Minor number) khá phức tạp.

Để đơn giản hóa, Linux cung cấp cơ chế **Misc Device** (định nghĩa trong `<linux/miscdevice.h>`):
- Tất cả các thiết bị Misc đều dùng chung Major number là **`10`**.
- Hệ thống tự động phân phối Minor number động thông qua hằng số `MISC_DYNAMIC_MINOR`.
- Khi gọi `misc_register()`, nhân Linux sẽ **tự động đăng ký và tạo file thiết bị tương ứng dưới thư mục `/dev/`** dựa trên tên cấu hình, giúp ta không cần gọi lệnh tạo file thủ công (`mknod`).

---

## 2. Cấu trúc File Operations trong Driver

Driver liên kết các hành vi hệ thống thông qua cấu trúc `struct file_operations`:
```c
static const struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .open = misc_open,       // Gọi khi ứng dụng open("/dev/misc-module")
    .release = misc_release, // Gọi khi ứng dụng close(fd)
    .read = misc_read,       // Gọi khi ứng dụng read(fd, ...)
    .write = misc_write,     // Gọi khi ứng dụng write(fd, ...)
    .llseek = misc_llseek,   // Gọi khi ứng dụng lseek(fd, ...)
};
```

---

## 3. Quy trình Biên dịch và Nạp Module

Để biên dịch Driver nhân Linux, bạn cần cài đặt gói biên dịch và bộ cài headers tương ứng với phiên bản nhân hiện tại của Linux:
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

### Cách chạy nhanh bằng Script
Di chuyển vào thư mục bài học và chạy lệnh:
```bash
bash restart.bash
```
*Script sẽ yêu cầu quyền root (sudo) để thực thi nạp module (`insmod`), chmod cho thiết bị, chạy thử test-app và tháo gỡ module (`rmmod`).*

### Các bước thực hiện thủ công bằng lệnh

1. **Biên dịch mã nguồn (Module + App):**
   ```bash
   make
   ```
   Lệnh này tạo ra tệp nhị phân Driver nhân: `misc-module.ko` và ứng dụng kiểm tra `test-app`.

2. **Nạp module vào nhân Linux:**
   ```bash
   sudo insmod misc-module.ko
   ```

3. **Xác nhận thiết bị đã được tạo tự động:**
   ```bash
   ls -la /dev/misc-module
   ```
   *Bạn sẽ thấy một file character device có Major number là 10.*

4. **Phân quyền truy cập thiết bị:**
   ```bash
   sudo chmod 666 /dev/misc-module
   ```

5. **Chạy ứng dụng kiểm chứng:**
   ```bash
   ./test-app
   ```

6. **Đọc log thông tin từ bên trong Nhân Linux:**
   Mỗi lệnh debug `pr_info()` trong driver sẽ được ghi nhận vào nhân. Bạn có thể đọc nó bằng:
   ```bash
   dmesg | tail -n 15
   ```

7. **Gỡ bỏ module ra khỏi nhân Linux:**
   ```bash
   sudo rmmod misc-module
   ```

8. **Dọn dẹp sản phẩm build:**
   ```bash
   make clean
   ```
