# Bài 5.2 - Giả lập Client-Server bằng Shared Memory & OS Signals

## Mục tiêu

Xây dựng mô hình tương tác Client-Server giả lập chạy trên 2 tiến trình độc lập không sử dụng `fork()`:
1. **Server và Client chia sẻ chung một vùng nhớ** chứa dữ liệu có cấu trúc:
   ```c
   struct data {
       char name[25];
       int tuoi;
   };
   ```
2. **Server dành hầu hết thời gian để ngủ (`pause()`)**, không tiêu tốn chu kỳ CPU.
3. **Client yêu cầu người dùng nhập thông tin Tên và Tuổi** từ bàn phím. Sau khi nhập xong và lưu vào vùng nhớ chia sẻ, Client sẽ gửi một **Tín hiệu hệ điều hành (SIGUSR1)** để đánh thức Server dậy.
4. **Server sau khi tỉnh dậy** sẽ lấy dữ liệu từ vùng nhớ chia sẻ, ghi nhật ký vào file `server.log`, rồi tiếp tục quay trở lại trạng thái ngủ.

---

## 1. Lý thuyết về Tín hiệu Hệ điều hành (OS Signals) trong Linux

### Tín hiệu (Signal) là gì?
**Signal** là một cơ chế ngắt phần mềm (software interrupt) được hệ điều hành Linux gửi tới một tiến trình nhằm thông báo về một sự kiện bất thường hoặc sự kiện do người dùng/tiến trình khác yêu cầu.

Trong bài học này, chúng ta sử dụng tín hiệu **`SIGUSR1`** (User-defined Signal 1) là tín hiệu dành riêng cho lập trình viên thiết lập các chức năng tùy biến giao tiếp giữa các tiến trình.

### Các hàm điều khiển tín hiệu quan trọng

1. **`signal()` (Đăng ký bộ xử lý ngắt):**
   ```c
   sighandler_t signal(int signum, sighandler_t handler);
   ```
   Dùng để thiết lập hàm xử lý (callback function) sẽ tự động được gọi khi tiến trình nhận được tín hiệu `signum`.

2. **`kill()` (Gửi tín hiệu):**
   ```c
   int kill(pid_t pid, int sig);
   ```
   Gửi tín hiệu `sig` tới tiến trình có mã định danh `pid`. Client dùng hàm này để gửi `SIGUSR1` tới Server.

3. **`pause()` (Ngủ tiết kiệm năng lượng):**
   ```c
   int pause(void);
   ```
   Tạm dừng tiến trình hiện tại cho đến khi nhận được một tín hiệu bất kỳ. Khi ngủ bằng `pause()`, hệ điều hành sẽ đưa tiến trình vào trạng thái **Sleep (S)**, giải phóng CPU hoàn toàn (CPU usage = 0%).

---

## 2. Cơ chế Đồng bộ hóa & Luồng Hoạt động

Để tránh việc Client nhập dữ liệu quá nhanh khiến Server chưa kịp ghi log đã bị ghi đè, hai tiến trình sử dụng hai cờ trạng thái `data_ready` và `processed` trong Shared Memory để đồng bộ:

```
    CLIENT                                                     SERVER
 ┌──────────┐                                               ┌──────────┐
 │  Nhập    │                                               │ Khởi tạo │
 │ Tên,Tuổi │                                               │  SHM &   │
 └────┬─────┘                                               │ Ghi PID  │
      │                                                     └────┬─────┘
  Ghi vào SHM                                                    │
  data_ready=1 ──────────────────────────────────────────────► pause() (Ngủ Zzz...)
  processed=0                                                    │
      │                                                          │
  kill(pid, SIGUSR1) ──[ Gửi tín hiệu đánh thức SIGUSR1 ]──────► Woke up! (Tỉnh dậy)
      │                                                          │
  Chờ phản hồi                                              Ghi vào log file
  (processed==1) ◄─────[ Đặt processed=1 và data_ready=0 ]──── Đọc SHM
      │                                                          │
 ┌────┴─────┐                                                    │
 │ Tiếp tục │◄───────────────────────────────────────────────────┘
 └──────────┘
```

---

## 3. Cấu trúc thư mục bài học

- **`shared_data.h`**: Định nghĩa cấu trúc `struct data`, cấu trúc `struct shared_data` (chứa thêm PID của server và các cờ đồng bộ), tên phân vùng `/shm_data_5_2` và tên file log `server.log`.
- **`server.c`**: Tiến trình Server tạo Shared Memory, viết PID của mình lên đó, thiết lập bộ xử lý `SIGUSR1`, ngủ bằng `pause()`. Khi tỉnh dậy, nó ghi log có kèm **timestamp** (thời gian thực) vào file `server.log`.
- **`client.c`**: Tiến trình Client mở Shared Memory, lấy PID của Server, nhập liệu trong vòng lặp an toàn, gửi tín hiệu đánh thức Server bằng lệnh `kill()`.
- **`restart.bash`**: Script biên dịch tự động và khởi chạy hai terminal độc lập để chạy Client và Server.

---

## 4. Hướng dẫn Biên dịch và Chạy thử

### Cách chạy nhanh bằng Script
Chạy lệnh sau trong thư mục bài học:
```bash
bash restart.bash
```

### Cách chạy thủ công bằng Terminal
Mở hai cửa sổ terminal Linux độc lập:

1. **Biên dịch mã nguồn:**
   ```bash
   # Biên dịch server (sử dụng thư viện Real-time -lrt cho Shared Memory)
   gcc -o server server.c -lrt
   
   # Biên dịch client
   gcc -o client client.c -lrt
   ```

2. **Chạy Server (Bắt buộc chạy trước để khởi tạo và ghi PID):**
   ```bash
   ./server
   ```
   Server sẽ in ra mã PID của nó và đi vào trạng thái ngủ `Zzz...`.

3. **Chạy Client:**
   ```bash
   ./client
   ```
   Client sẽ tự động kết nối và hiển thị PID của Server mà nó tìm thấy trong Shared Memory.

4. **Thử nghiệm nhập liệu:**
   - Tại Client, nhập tên: `Nguyen Van A`
   - Nhập tuổi: `20`
   - Quan sát Client thông báo đã gửi tín hiệu, Server ở cửa sổ bên kia lập tức thức dậy, ghi nhận dữ liệu, ghi log và quay lại đi ngủ tiếp.

5. **Xem nội dung file log:**
   Mở file log để kiểm tra kết quả lưu trữ:
   ```bash
   cat server.log
   ```
   Định dạng file log sẽ hiển thị chi tiết:
   ```text
   [2026-06-09 16:15:00] [INFO] Nhan du lieu: Ten = Nguyen Van A, Tuoi = 20
   ```

6. **Thoát chương trình:**
   - Tại Client, gõ `/quit` hoặc ấn `Ctrl+C` để thoát.
   - Tại Server, ấn `Ctrl+C`. Bộ ngắt tín hiệu `SIGINT` đã được đăng ký để tự động xóa sạch Shared Memory ảo `/dev/shm/shm_data_5_2` khỏi nhân Linux.

---

## 5. Các lưu ý nâng cao về an toàn lập trình C trong Linux

1. **`volatile sig_atomic_t`**:
   Biến `sigusr1_received` dùng để giao tiếp giữa hàm xử lý tín hiệu và luồng chính của Server. Nó bắt buộc phải khai báo bằng kiểu `volatile sig_atomic_t` để đảm bảo hệ điều hành truy cập trực tiếp vào RAM (không lưu đệm trong thanh ghi CPU) và thao tác đọc/ghi biến này diễn ra đơn nguyên (atomic), tránh lỗi xung đột dữ liệu khi bị ngắt.

2. **Mức độ chiếm dụng CPU của `pause()`**:
   So với việc dùng vòng lặp bận (busy-waiting) để kiểm tra biến cờ `while(!data_ready) {}` (khiến CPU hoạt động 100%), lệnh `pause()` đưa tiến trình vào danh sách chờ của nhân Linux (mức độ ưu tiên thấp nhất), giúp tiết kiệm tài nguyên hệ thống triệt để.

3. **Kiểm tra rò rỉ bộ nhớ chia sẻ:**
   Bạn có thể kiểm tra xem file đại diện cho bộ nhớ chia sẻ có được tạo ra trong `/dev/shm` hay không bằng lệnh:
   ```bash
   ls -la /dev/shm/shm_data_5_2
   ```
