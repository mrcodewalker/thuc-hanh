# Bài 6.1 - Đọc ghi File từ Vị trí Bất kỳ (Lseek)

## Mục tiêu

Viết một chương trình C tự tạo một file text có tên `sample.txt`, ghi nội dung mẫu vào đó, sau đó sử dụng lời gọi hệ thống di chuyển con trỏ tệp tin (**`lseek()`**) để đọc nội dung từ một vị trí bất kỳ và hiển thị kết quả ra màn hình.

---

## 1. Các lời gọi hệ thống (System Calls) thao tác File trong Linux

Hệ điều hành Linux quản lý tệp tin thông qua các File Descriptor (mô tả tệp tin - là một số nguyên dương đại diện cho luồng I/O). Các hàm cơ bản gồm:

### `open()` - Mở hoặc tạo file
```c
int open(const char *pathname, int flags, mode_t mode);
```
- **`flags`**: Chế độ mở file:
  - `O_RDONLY`: Chỉ đọc.
  - `O_WRONLY`: Chỉ ghi.
  - `O_CREAT`: Tạo file nếu chưa tồn tại.
  - `O_TRUNC`: Xoá dữ liệu cũ nếu file đã tồn tại.
- **`mode`**: Quyền truy cập file khi tạo mới (ví dụ: `0644` là chủ sở hữu được đọc/ghi, nhóm và người khác chỉ được đọc).

### `write()` - Ghi dữ liệu vào file
```c
ssize_t write(int fd, const void *buf, size_t count);
```
- Ghi `count` byte từ bộ đệm `buf` vào file mô tả bởi `fd`. Trả về số byte ghi thực tế hoặc `-1` nếu lỗi.

### `read()` - Nhận dữ liệu từ file
```c
ssize_t read(int fd, void *buf, size_t count);
```
- Đọc `count` byte từ file `fd` đưa vào bộ đệm `buf`. Trả về số byte đọc thực tế, `0` nếu gặp EOF (kết thúc file), hoặc `-1` nếu lỗi.

### `lseek()` - Định vị con trỏ tệp tin
```c
off_t lseek(int fd, off_t offset, int whence);
```
- Di chuyển con trỏ vị trí đọc/ghi của file `fd` đi một khoảng `offset` dựa trên mốc `whence`:
  - **`SEEK_SET`**: Mốc bắt đầu từ đầu file. Vị trí mới = `offset`.
  - **`SEEK_CUR`**: Mốc từ vị trí hiện tại của con trỏ. Vị trí mới = Vị trí hiện tại + `offset`.
  - **`SEEK_END`**: Mốc từ cuối file. Vị trí mới = Kích thước file + `offset`.

### `close()` - Đóng file descriptor
```c
int close(int fd);
```
- Giải phóng file descriptor để hệ thống tái sử dụng.

---

## 2. Hướng dẫn Biên dịch và Chạy

### Cách chạy nhanh bằng Script
Di chuyển vào thư mục bài học và chạy lệnh:
```bash
bash restart.bash
```
*Script sẽ tự động biên dịch chương trình và chạy thử hai lần: lần một với tham số mặc định và lần hai với tham số tuỳ chỉnh.*

### Cách biên dịch thủ công
1. **Biên dịch:**
   ```bash
   gcc -o read_anywhere read_anywhere.c
   ```
2. **Chạy mặc định (Đọc từ vị trí byte 14, độ dài 20 ký tự):**
   ```bash
   ./read_anywhere
   ```
3. **Chạy tuỳ chọn (Đọc từ vị trí byte 22, độ dài 15 ký tự):**
   ```bash
   ./read_anywhere 22 15
   ```
   *Cú pháp rộng hơn:* `./read_anywhere <vị_trí_offset> <độ_dài_cần_đọc>`
