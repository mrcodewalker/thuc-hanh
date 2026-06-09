# Bài 6.4 - Tạo Thư mục Cấu trúc và Lọc File Read-only

## Mục tiêu

Sử dụng ngôn ngữ C để tạo ra một cấu trúc thư mục phân cấp chứa nhiều tệp tin có quyền truy cập (mode) khác nhau. Sau đó, viết thuật toán duyệt thư mục đệ quy để tự động tìm kiếm, phân tích mặt nạ quyền và hiển thị danh sách các tệp tin có trạng thái **Read-only** (Chỉ cho phép đọc - không có quyền ghi đối với mọi nhóm đối tượng).

---

## 1. Lý thuyết quyền truy cập tệp tin (File Mode) trong Linux

Trong Linux, mỗi tệp tin/thư mục liên kết với một mặt nạ quyền truy cập nhị phân dài 9 bit, chia thành 3 nhóm đối tượng:
- **Owner (Người sở hữu):** Ký hiệu `u`, quản lý bởi các hằng số `S_IRUSR` (đọc), `S_IWUSR` (ghi), `S_IXUSR` (thực thi).
- **Group (Nhóm):** Ký hiệu `g`, quản lý bởi `S_IRGRP`, `S_IWGRP`, `S_IXGRP`.
- **Others (Người khác):** Ký hiệu `o`, quản lý bởi `S_IROTH`, `S_IWOTH`, `S_IXOTH`.

Biểu diễn hệ Bát phân (Octal) của các quyền:
- **Đọc (Read):** Giá trị nhị phân `100` $\rightarrow$ Số bát phân là **`4`**.
- **Ghi (Write):** Giá trị nhị phân `010` $\rightarrow$ Số bát phân là **`2`**.
- **Thực thi (Execute):** Giá trị nhị phân `001` $\rightarrow$ Số bát phân là **`1`**.

Các ví dụ về Mode phổ biến:
- `0755` (`rwxr-xr-x`): Đầy đủ quyền cho Owner; Đọc/Thực thi cho Group và Others.
- `0644` (`rw-r--r--`): Đọc/Ghi cho Owner; Chỉ đọc cho Group và Others.
- `0444` (`r--r--r--`): Chỉ đọc cho tất cả các bên.
- `0400` (`r--------`): Chỉ đọc cho riêng Owner, các bên khác bị cấm hoàn toàn.
- `0222` (`-w--w--w-`): Chỉ ghi cho tất cả các bên.

---

## 2. Lưu ý kỹ thuật về Umask và Hàm `chmod()`

Khi tạo tệp tin bằng hàm `open(path, O_CREAT, mode)`, quyền thực tế của file được tạo ra sẽ bị thay đổi bởi mặt nạ mặc định của hệ thống gọi là **`umask`** theo công thức:
$$\text{Quyền thực tế} = \text{mode} \ \text{AND} \ (\text{NOT} \ \text{umask})$$

Ví dụ, nếu `umask` hệ thống đang là `0022` và ta yêu cầu tạo file với mode `0666`, hệ thống sẽ tạo ra file có mode `0644`.

Để ghi đè hoàn toàn bộ lọc `umask` và thiết lập quyền truy cập tuyệt đối chính xác cho bài tập này, sau khi tạo tệp, chương trình C thực hiện gọi hàm **`chmod()`**:
```c
int chmod(const char *pathname, mode_t mode);
```
Hàm này sẽ ghi đè trực tiếp chế độ phân quyền lên tệp tin bất chấp cấu hình `umask` trước đó.

---

## 3. Thuật toán lọc file Read-only

Thuật toán duyệt qua cấu trúc thư mục đệ quy sử dụng các cuộc gọi hệ thống `readdir()` và `stat()`. Một tệp tin thường (`S_ISREG`) được phân loại là **Read-only** khi đáp ứng đồng thời 2 điều kiện:

1. **Có quyền đọc:** Có ít nhất một bit đọc được kích hoạt ở Owner, Group hoặc Others:
   ```c
   int is_read_enabled = ((st.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)) != 0);
   ```
2. **Không có quyền ghi:** Hoàn toàn không có bất kỳ bit ghi nào được bật ở cả 3 nhóm đối tượng:
   ```c
   int is_write_disabled = ((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0);
   ```

---

## 4. Cấu trúc thư mục thử nghiệm được tạo ra

Khi chạy chương trình, cấu trúc thư mục sau sẽ được dựng lên:
```
test_structure/
├── file_read_write.txt    (0644 - Sẽ bị bỏ qua vì có quyền ghi cho Owner)
├── file_read_only.txt     (0444 - Read-only cho tất cả - Sẽ được chọn)
├── file_write_only.txt    (0222 - Bị bỏ qua vì có quyền ghi và không có quyền đọc)
└── sub_folder/
    ├── file_ro_owner.txt  (0400 - Read-only cho Owner - Sẽ được chọn)
    └── file_executable.sh (0755 - Bị bỏ qua vì có quyền ghi cho Owner)
```

---

## 5. Hướng dẫn Biên dịch và Chạy

### Cách chạy nhanh bằng Script
Di chuyển vào thư mục bài học và chạy lệnh:
```bash
bash restart.bash
```
*Script sẽ tự động dọn dẹp thư mục cũ, biên dịch chương trình, chạy thử quét ra các file Read-only, và cuối cùng chạy lệnh hệ thống `ls -laR` để in ra phân quyền thực tế cho bạn đối chứng.*

### Cách biên dịch thủ công
1. **Biên dịch:**
   ```bash
   gcc -o manage_dir manage_dir.c
   ```
2. **Chạy chương trình:**
   ```bash
   ./manage_dir
   ```
