# Bài 5.1 - Chat Client-Server bằng Shared Memory (POSIX)

## Mục tiêu

Viết lại ứng dụng chat giữa **Server** và **Client** chạy trên **cùng một máy tính**, sử dụng **Shared Memory (Bộ nhớ chia sẻ)** để giao tiếp thay thế cho Socket. Chương trình hỗ trợ giao tiếp hai chiều đồng thời (Full-duplex) và xử lý đồng bộ hóa tài nguyên bằng **POSIX Semaphores**.

---

## 1. Khái niệm Shared Memory trong lập trình hệ thống Linux

### Shared Memory là gì?
**Shared Memory (Bộ nhớ chia sẻ)** là một trong những cơ chế giao tiếp giữa các tiến trình (IPC - Inter-Process Communication) nhanh nhất trong hệ điều hành Linux/Unix. 

Thông thường, mỗi tiến trình chạy trong một không gian địa chỉ ảo độc lập và được bảo vệ bởi hệ điều hành để tránh xâm phạm lẫn nhau. Tuy nhiên, với Shared Memory, hệ điều hành sẽ ánh xạ một vùng nhớ vật lý dùng chung vào không gian địa chỉ ảo của cả hai tiến trình:

```
┌─────────────────┐                     ┌─────────────────┐
│ Tiến trình A    │                     │ Tiến trình B    │
│ (Địa chỉ ảo A)  │                     │ (Địa chỉ ảo B)  │
└────────┬────────┘                     └────────┬────────┘
         │                                       │
         └─────────────► [VÙNG NHỚ VẬT LÝ] ◄─────┘
                         (Shared Memory)
```

### Tại sao Shared Memory là cơ chế IPC nhanh nhất?
- **Không sao chép dữ liệu qua Kernel:** Đối với Pipe, Message Queue hoặc Socket, dữ liệu phải được sao chép từ không gian người dùng (User Space) của Tiến trình gửi $\rightarrow$ Không gian nhân (Kernel Space) $\rightarrow$ Không gian người dùng của Tiến trình nhận. Đối với Shared Memory, tiến trình viết trực tiếp vào vùng nhớ chung, tiến trình kia đọc ngay lập tức từ đó.
- **Tốc độ truy cập:** Tương đương tốc độ đọc/ghi RAM thông thường của một tiến trình độc lập.

### Các hàm POSIX Shared Memory API quan trọng

Để sử dụng bộ nhớ chia sẻ POSIX trên Linux, cần liên kết với thư viện Real-time (`-lrt`).

| Hàm | Cú pháp chính | Mô tả |
| :--- | :--- | :--- |
| **`shm_open()`** | `shm_open(const char *name, int oflag, mode_t mode)` | Tạo mới hoặc mở một phân vùng bộ nhớ chia sẻ. Trả về một *File Descriptor*. |
| **`ftruncate()`**| `ftruncate(int fd, off_t length)` | Thiết lập kích thước (byte) cho phân vùng bộ nhớ chia sẻ. |
| **`mmap()`** | `mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)` | Ánh xạ file descriptor của bộ nhớ chia sẻ vào vùng nhớ ảo của tiến trình. |
| **`munmap()`** | `munmap(void *addr, size_t len)` | Hủy ánh xạ bộ nhớ chia sẻ khỏi vùng địa chỉ ảo của tiến trình. |
| **`shm_unlink()`**| `shm_unlink(const char *name)` | Xóa bỏ tên phân vùng bộ nhớ chia sẻ khỏi hệ thống (khi không còn tiến trình nào dùng, nó sẽ giải phóng vật lý). |

---

## 2. Cơ chế đồng bộ hóa bằng POSIX Semaphores

Vì Shared Memory cho phép các tiến trình truy cập đồng thời, nếu không đồng bộ hóa sẽ dẫn đến lỗi **Race Condition** (Xung đột ghi đè, đọc dữ liệu rác, ghi đè tin nhắn chưa đọc).

Bài này sử dụng các **Unnamed Semaphores** đặt trực tiếp bên trong cấu trúc Shared Memory và được cấu hình chia sẻ giữa các tiến trình (`pshared = 1`).

### Các hàm Semaphore quan trọng

| Hàm | Mô tả |
| :--- | :--- |
| **`sem_init(sem_t *sem, int pshared, unsigned int val)`** | Khởi tạo semaphore. Đặt `pshared = 1` để chia sẻ giữa các tiến trình khác nhau. |
| **`sem_wait(sem_t *sem)`** | Thao tác P (khóa). Nếu giá trị semaphore > 0, giảm đi 1 và đi tiếp. Nếu = 0, block tiến trình cho đến khi được post. |
| **`sem_post(sem_t *sem)`** | Thao tác V (mở khóa). Tăng giá trị semaphore lên 1, đánh thức tiến trình đang bị block ở `sem_wait`. |
| **`sem_destroy(sem_t *sem)`** | Hủy semaphore và giải phóng tài nguyên hệ thống liên quan. |

---

## 3. Kiến trúc thiết kế chương trình

### Cấu trúc dữ liệu dùng chung (`shared_msg.h`)
Hai tiến trình giao tiếp qua phân vùng bộ nhớ chia sẻ định nghĩa bởi cấu trúc `shared_data`:

```c
struct chat_channel {
    char message[1024];
    sem_t sem_filled;  // Báo hiệu tin nhắn mới đã được ghi (ban đầu = 0)
    sem_t sem_empty;   // Báo hiệu bộ đệm đã trống và sẵn sàng ghi (ban đầu = 1)
};

struct shared_data {
    struct chat_channel s2c;   // Kênh Server gửi -> Client đọc
    struct chat_channel c2s;   // Kênh Client gửi -> Server đọc
    volatile int server_alive; // Cờ báo Server đang chạy
    volatile int client_alive; // Cờ báo Client đang chạy
};
```

### Luồng đồng bộ hóa dữ liệu (Producer-Consumer)
Quy trình gửi nhận tin nhắn diễn ra theo chu kỳ đóng mở semaphore:

```
[Tiến trình Gửi]                       [Tiến trình Nhận]
       │                                       │
sem_wait(&sem_empty)  ← Chờ bộ đệm trống       │
       │                                       │
  (Ghi tin nhắn)                               │
       │                                       │
sem_post(&sem_filled) ────────────────► sem_wait(&sem_filled)  ← Chờ tin nhắn mới
                                               │
                                         (Đọc tin nhắn)
                                               │
                                        sem_post(&sem_empty)   ← Báo đã đọc xong
```

---

## 4. Kiến trúc Đa luồng (Multi-threading) & Nhập liệu Phi chặn (Non-blocking Input)

Cả **Server** và **Client** đều chạy song song hai luồng xử lý:

```
┌────────────────────────────────────────────────────────┐
│                   TIẾN TRÌNH CHAT                      │
├────────────────────────────────────────────────────────┤
│  Thread chính (Gửi):                                   │
│    Vòng lặp:                                           │
│      1. Dùng select() thăm dò STDIN (timeout 100ms)    │
│      2. Nếu có dữ liệu nhập:                           │
│           sem_wait(empty) -> ghi tin -> sem_post(filled)│
│      3. Nếu timeout:                                   │
│           Kiểm tra cờ alive đối phương để tự thoát    │
│                                                        │
│  Thread phụ (Nhận):                                    │
│    Vòng lặp:                                           │
│      1. sem_wait(filled)                               │
│      2. Đọc tin -> In ra màn hình                      │
│      3. sem_post(empty)                                │
└────────────────────────────────────────────────────────┘
```

**Tại sao dùng `select()` trên `stdin`?**
Nếu dùng `fgets()` thông thường, luồng chính sẽ bị **block** hoàn toàn tại `stdin` cho đến khi người dùng gõ Enter. Nếu đối phương ngắt kết nối đột ngột (hoặc tắt chương trình), tiến trình hiện tại sẽ không thể phát hiện cờ `alive == 0` để tự động thoát. 
Sử dụng `select()` với timeout 100ms giải quyết triệt để vấn đề này, giúp chương trình phản hồi cực nhanh khi có sự cố ngắt kết nối.

---

## 5. Hướng dẫn Biên dịch và Chạy

### Cách chạy nhanh bằng Script
Chạy script tự động biên dịch, dọn dẹp bộ nhớ chia sẻ cũ và mở 2 terminal để chat:
```bash
bash restart.bash
```

### Cách chạy thủ công bằng Terminal
Nếu không tìm thấy công cụ giả lập terminal để mở tự động, hãy mở 2 terminal khác nhau và thực hiện:

1. **Biên dịch mã nguồn:**
   ```bash
   # Biên dịch server (cần -lpthread cho luồng và -lrt cho shared memory)
   gcc -o server server.c -lpthread -lrt
   
   # Biên dịch client
   gcc -o client client.c -lpthread -lrt
   ```

2. **Chạy Server (phải chạy Server trước để khởi tạo Shared Memory):**
   ```bash
   ./server
   ```

3. **Chạy Client:**
   ```bash
   ./client
   ```

### Hướng dẫn sử dụng
- Gõ nội dung tin nhắn và nhấn **Enter** để gửi.
- Gõ `/quit` ở bất kỳ bên nào để ngắt kết nối và thoát toàn bộ hệ thống trò chuyện một cách an toàn.
- Bạn cũng có thể dùng tổ hợp phím **Ctrl+C**. Bộ xử lý tín hiệu `SIGINT` đã được thiết lập để thu hồi bộ nhớ chia sẻ `/dev/shm/shm_chat_5_1` sạch sẽ trước khi thoát.

---

## 6. Lưu ý kỹ thuật nâng cao trong Nhân Linux

1. **Định vị Shared Memory ảo:**
   Trên Linux, các phân vùng Shared Memory POSIX thực chất được ánh xạ thành các file ảo trong thư mục `/dev/shm/`. Sau khi chạy server, bạn có thể kiểm tra sự tồn tại của file này bằng lệnh:
   ```bash
   ls -l /dev/shm/shm_chat_5_1
   ```
2. **Cờ khởi tạo Semaphore:**
   `sem_init(&sem, 1, val)`: Tham số thứ hai bằng `1` (pshared) chỉ định semaphore này nằm trong bộ nhớ chia sẻ và được dùng chung giữa các tiến trình riêng biệt. Nếu đặt bằng `0`, semaphore chỉ có tác dụng giữa các thread trong cùng một tiến trình.
3. **EINTR (Interrupted System Call):**
   Khi một tiến trình đang bị block bởi `sem_wait` hoặc `select`, nếu có một tín hiệu (như `SIGINT` khi bấm Ctrl+C) gửi đến, các hàm này sẽ bị đánh thức và trả về lỗi `-1` với `errno` đặt là `EINTR`. Code đã được cài đặt để nhận biết trạng thái này nhằm thoát vòng lặp an toàn mà không bị crash.
