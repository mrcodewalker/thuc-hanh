# Bài 6.5 - Đóng gói ứng dụng, viết chương trình Setup & Uninstall bằng C

## Mục tiêu

Sử dụng **Makefile** để biên dịch ứng dụng chat Shared Memory thành một bộ sản phẩm đóng gói hoàn chỉnh (**Packet**), bao gồm:
1. File thực thi (**Binaries**): `chat-server` và `chat-client`.
2. Thư viện chia sẻ (**Shared Library**): `libchat.so` chứa mã nguồn đọc file cấu hình và ghi log.
3. File cấu hình (**Config file**): `chat.conf` dùng để tùy chỉnh tên Shared Memory và file log.

Đồng thời, viết hai chương trình hoàn toàn bằng ngôn ngữ C sử dụng các lời gọi hệ thống của Linux:
- **`setup.c`**: Chương trình cài đặt, tự động đưa các file vào đúng vị trí đích hệ thống và phân quyền thích hợp.
- **`uninstall.c`**: Chương trình gỡ cài đặt, tự động xóa sạch các file đã cài và khôi phục trạng thái ban đầu của hệ thống.

---

## 1. Thiết kế Gói cài đặt và Vị trí phân bổ trên Hệ thống

Theo tiêu chuẩn phân cấp thư mục của Linux (FHS - Filesystem Hierarchy Standard), các thành phần của ứng dụng sẽ được phân bổ như sau:

| Thành phần | File nguồn | Thư mục cài đặt đích | Phân quyền (Mode) |
| :--- | :--- | :--- | :--- |
| **Ứng dụng chạy (Server)** | `packet/bin/chat-server` | `/usr/local/bin/chat-server` | `0755` (Đọc/Ghi/Chạy cho Root, Đọc/Chạy cho User) |
| **Ứng dụng chạy (Client)** | `packet/bin/chat-client` | `/usr/local/bin/chat-client` | `0755` |
| **Thư viện chia sẻ** | `packet/lib/libchat.so` | `/usr/local/lib/libchat.so` | `0755` |
| **Tệp cấu hình** | `packet/config/chat.conf` | `/etc/chat.conf` | `0644` (Đọc/Ghi cho Root, Chỉ đọc cho User) |
| **Nhật ký chat (Log)** | Tự động sinh khi chạy | `/var/log/chat_app.log` | `0666` (Để tiến trình User có quyền ghi) |

---

## 2. Các điểm kỹ thuật cốt lõi trong Nhân Linux & Lập trình C

### Biên dịch Thư viện chia sẻ (`libchat.so`)
Thư viện chia sẻ là tập hợp mã nguồn có thể được nạp động vào bộ nhớ lúc chạy bởi nhiều tiến trình khác nhau.
- Cờ **`-fPIC`** (Position Independent Code): Tạo mã nguồn độc lập vị trí địa chỉ, bắt buộc khi build Shared Library.
- Cờ **`-shared`**: Chỉ định xuất ra file `.so` thay vì file thực thi.

### Chỉ định RPATH khi liên kết (Linker)
Khi Server và Client chạy, trình nạp liên kết động (dynamic linker) của Linux cần biết tìm file `libchat.so` ở đâu. Ta cấu hình cờ biên dịch:
`-Wl,-rpath,/usr/local/lib -Wl,-rpath,.`
- `-Wl,...`: Chuyển tùy chọn trực tiếp đến trình liên kết (linker).
- `-rpath`: Ghi đè đường dẫn tìm kiếm thư viện động vào trực tiếp trong tiêu đề của tệp nhị phân binary.
  - `/usr/local/lib`: Giúp chương trình tìm thấy thư viện sau khi cài đặt.
  - `.`: Giúp chương trình chạy thử cục bộ tại thư mục hiện tại mà không bị lỗi.

### Sao chép và Phân quyền file trong `setup.c`
Chương trình `setup` kiểm tra quyền root thông qua `getuid() == 0` (chỉ root mới được ghi vào `/usr/local/` và `/etc/`).
- Việc copy file được triển khai bằng các thao tác byte nhị phân trong C (`fread`/`fwrite`).
- Cài đặt phân quyền bằng cách gọi hàm hệ thống `chmod()`.
- Cập nhật lại cache thư viện liên kết động của Linux bằng cách gọi `system("ldconfig")`.

### Gỡ bỏ tệp tin bằng hệ thống gọi `unlink()` trong `uninstall.c`
Trong Linux, lệnh xoá file thực chất là xoá liên kết (link) của tên file đó tới node dữ liệu vật lý (inode). Chương trình `uninstall` thực hiện việc này một cách trực tiếp ở cấp độ hệ thống thông qua:
```c
int unlink(const char *pathname);
```
Sau đó gọi `ldconfig` để xóa thư viện khỏi danh sách cache liên kết.

---

## 3. Hướng dẫn Biên dịch và Cài đặt

### Cách chạy nhanh bằng Script
Di chuyển vào thư mục bài học và chạy lệnh:
```bash
bash restart.bash
```
*Script sẽ dọn dẹp các tệp cũ, gọi `make` để đóng gói bộ `packet/`, và in ra hướng dẫn chi tiết cách bạn cài đặt/chạy thử.*

### Quy trình chạy thủ công từng bước

1. **Biên dịch và Đóng gói (Make):**
   ```bash
   make
   ```
   Lệnh này sẽ tự động tạo thư mục `packet` chứa toàn bộ các file cần thiết của app, đồng thời tạo ra hai file cài đặt `setup` và gỡ cài đặt `uninstall`.

2. **Tiến hành cài đặt:**
   ```bash
   sudo ./setup
   ```
   *Chương trình sẽ copy các file vào hệ thống và đăng ký thư viện động.*

3. **Chạy thử ứng dụng từ bất kỳ đâu (do đã cài vào `/usr/local/bin`):**
   Mở hai terminal độc lập:
   - **Terminal 1:**
     ```bash
     chat-server
     ```
   - **Terminal 2:**
     ```bash
     chat-client
     ```

4. **Kiểm tra file cấu hình:**
   Bạn có thể thay đổi tên phân vùng Shared Memory hoặc vị trí lưu file log tại:
   ```bash
   sudo nano /etc/chat.conf
   ```

5. **Đọc tệp log ghi chép cuộc hội thoại:**
   Mở file log hệ thống xem nội dung đã được ghi nhận:
   ```bash
   cat /var/log/chat_app.log
   ```

6. **Gỡ bỏ cài đặt khỏi hệ thống:**
   Sau khi hoàn thành bài thực hành, hãy dọn dẹp hệ thống bằng lệnh:
   ```bash
   sudo ./uninstall
   ```
   *Hệ thống sẽ hoàn toàn sạch sẽ như ban đầu.*
