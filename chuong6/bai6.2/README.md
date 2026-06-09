# Bài 6.2 - Liệt kê Thư mục bằng Readdir và Stat

## Mục tiêu

Sử dụng hàm **`readdir()`** kết hợp với lời gọi hệ thống **`stat()`** để duyệt qua danh sách các tệp tin trong một thư mục được cấu hình cố định trong mã nguồn. In ra thông tin chi tiết của từng tệp tin dưới dạng bảng, bao gồm:
1. Phân loại tệp (FOLDER, FILE, LINK,...)
2. Tên tệp tin
3. Kích thước (dung lượng tính bằng byte)
4. Thời gian sửa đổi gần nhất (Modify time)

---

## 1. Các APIs quản lý thư mục và trạng thái tệp tin trên Linux

### Thao tác với Thư mục (`<dirent.h>`)

1. **`opendir()` - Mở luồng thư mục:**
   ```c
   DIR *opendir(const char *name);
   ```
   Mở một thư mục tại đường dẫn `name` và trả về con trỏ quản lý luồng thư mục `DIR`.

2. **`readdir()` - Đọc mục tin trong thư mục:**
   ```c
   struct dirent *readdir(DIR *dirp);
   ```
   Mỗi lần gọi, hàm trả về một cấu trúc `struct dirent` đại diện cho một mục tin (file, thư mục con) kế tiếp trong thư mục. Trả về `NULL` khi đã duyệt hết.
   Cấu trúc chứa trường quan trọng:
   - `char d_name[]`: Tên của mục tin đó.

3. **`closedir()` - Đóng luồng thư mục:**
   ```c
   int closedir(DIR *dirp);
   ```
   Đóng luồng thư mục sau khi làm việc xong để giải phóng tài nguyên.

---

### Truy vấn siêu dữ liệu tệp tin (`<sys/stat.h>`)

Hàm **`stat()`** dùng để đọc thông tin chi tiết về thuộc tính của một tệp tin thông qua đường dẫn:
```c
int stat(const char *pathname, struct stat *statbuf);
```
Dữ liệu trả về được ghi vào cấu trúc `struct stat` chứa các trường:
- **`st_size`**: Kích thước tệp tính bằng byte (kiểu `off_t`).
- **`st_mtime`**: Thời gian sửa đổi tệp lần cuối (kiểu `time_t`).
- **`st_mode`**: Kiểu tệp và quyền truy cập. Chúng ta dùng các macro sau để phân loại tệp tin:
  - `S_ISDIR(st_mode)`: Trả về true nếu là Thư mục.
  - `S_ISREG(st_mode)`: Trả về true nếu là Tệp thông thường.
  - `S_ISLNK(st_mode)`: Trả về true nếu là Liên kết mềm (Symbolic Link).

---

## 2. Hướng dẫn Biên dịch và Chạy

### Cách chạy nhanh bằng Script
Di chuyển vào thư mục bài học và chạy lệnh:
```bash
bash restart.bash
```
*Script sẽ tự động biên dịch chương trình và quét thư mục mặc định `.` (thư mục hiện tại) và thư mục cha `..`.*

### Cách biên dịch thủ công
1. **Biên dịch:**
   ```bash
   gcc -o list_dir list_dir.c
   ```
2. **Chạy mặc định (Quét thư mục cố định `.` trong mã nguồn):**
   ```bash
   ./list_dir
   ```
3. **Chạy quét một thư mục bất kỳ (ví dụ thư mục hệ thống `/var/log`):**
   ```bash
   ./list_dir /var/log
   ```
   *Cú pháp mở rộng:* `./list_dir <đường_dẫn_thư_mục_muốn_quét>`
