# Bài 5.0 - Chat Client-Server bằng mmap (Lưu lịch sử vào File)

## Mục tiêu

Xây dựng ứng dụng chat giữa **Server** và **Client** chạy trên cùng một máy tính, giao tiếp thông qua cơ chế ánh xạ tệp tin bộ nhớ (**File-backed memory mapping - `mmap`**). Toàn bộ nội dung trò chuyện sẽ tự động được lưu trữ bền vững vào tệp tin `chat_history.dat`. Khi khởi động lại ứng dụng, lịch sử trò chuyện cũ sẽ được tự động tải lại và hiển thị lên màn hình.

---

## 1. Lý thuyết ánh xạ bộ nhớ `mmap` trong Linux

### `mmap()` là gì?
`mmap()` là một lời gọi hệ thống (system call) cực kỳ mạnh mẽ trong Linux cho phép ánh xạ trực tiếp nội dung của một tệp tin (hoặc một thiết bị) vào không gian địa chỉ ảo của tiến trình. Sau khi ánh xạ, việc đọc/ghi trên file tương đương với việc đọc/ghi trên các con trỏ bộ nhớ thông thường.

### Phân loại ánh xạ mmap

| Tiêu chí | Ánh xạ liên kết file (File-backed Mapping) | Ánh xạ ẩn danh (Anonymous Mapping) |
| :--- | :--- | :--- |
| **Liên kết tệp** | Ánh xạ trực tiếp từ một tệp vật lý trên đĩa cứng (Ví dụ: `chat_history.dat`). | Không liên kết với bất kỳ tệp tin nào trên ổ đĩa. |
| **Độ bền vững** | **Bền vững (Persistent):** Dữ liệu được lưu trữ lại trên đĩa cứng ngay cả khi tắt nguồn, khởi động lại. | **Tạm thời (Volatile):** Dữ liệu biến mất hoàn toàn khi tiến trình thoát hoặc hệ thống tắt. |
| **Ứng dụng** | Cơ sở dữ liệu chat, quản lý file database (như SQLite, LMDB). | Cấp phát bộ nhớ động lớn (hàm `malloc()` gọi `mmap` ẩn danh). |

### Cờ `MAP_SHARED` và cơ chế ghi file tự động
Khi gọi `mmap`, cờ **`MAP_SHARED`** chỉ định rằng các sửa đổi trên vùng nhớ ảo sẽ được chia sẻ trực tiếp với tất cả các tiến trình khác cùng ánh xạ file này. Đồng thời, hệ điều hành sẽ tự động đồng bộ (flush) các trang nhớ bị sửa đổi (dirty pages) xuống tệp tin vật lý dưới đĩa cứng.

### Hàm `msync()` - Đảm bảo an toàn dữ liệu
Để đảm bảo dữ liệu chắc chắn được ghi xuống đĩa cứng một cách lập tức trước khi tiến trình đóng file hoặc thoát đột ngột, ta gọi hàm:
```c
int msync(void *addr, size_t length, int flags);
```
Sử dụng cờ `MS_SYNC` để yêu cầu hệ thống đồng bộ hóa cưỡng bức ngay tức thì (đồng bộ chặn) vùng nhớ về ổ đĩa vật lý.

---

## 2. Thiết kế Cơ sở dữ liệu và Đồng bộ hóa

### Định nghĩa độ dài cố định ghi vào Shared Memory
Vì `mmap` hoạt động trên không gian địa chỉ ảo ánh xạ trực tiếp từ file vật lý, kích thước file phải được định sẵn và cố định ngay từ đầu bằng hàm `ftruncate()`. Do đó, cấu trúc dữ liệu cơ sở dữ liệu chat `chat_db` được thiết kế có kích thước cố định như sau:

```c
#define MAX_MSG_LEN 256
#define MAX_MESSAGES 200

struct chat_message {
    char sender[10];      // "Server" hoặc "Client"
    char text[MAX_MSG_LEN]; // Tin nhắn có độ dài tối đa cố định
};

struct chat_db {
    struct chat_message messages[MAX_MESSAGES]; // Mảng tin nhắn cố định
    int msg_count;             // Tổng số tin nhắn đã lưu
    volatile int server_alive;
    volatile int client_alive;
    
    sem_t mutex;          // Loại trừ tương hỗ khi ghi vào mảng messages
    sem_t sem_server_new; // Server báo hiệu tin mới để Client đọc
    sem_t sem_client_new; // Client báo hiệu tin mới để Server đọc
};
```

### Đồng bộ hóa qua Semaphore độc lập
Nhằm tránh xung đột tín hiệu báo thức giữa Server và Client, cấu trúc lưu trữ tích hợp hai semaphores riêng biệt:
- `sem_server_new`: Server tăng lên khi viết tin nhắn mới $\rightarrow$ Client bị block ở đây sẽ thức giấc để nhận tin.
- `sem_client_new`: Client tăng lên khi viết tin nhắn mới $\rightarrow$ Server bị block ở đây sẽ thức giấc để nhận tin.
- `mutex`: Đảm bảo tại một thời điểm chỉ có duy nhất một tiến trình thực hiện ghi tin nhắn vào mảng và tăng biến đếm `msg_count`.

---

## 3. Quy trình Tải lịch sử trò chuyện (Load History)

```
 [Khởi động Server]
        │
Mở file chat_history.dat
        │
Đọc kích thước file bằng fstat()
        │
        ├──────► Kích thước = 0 (Tệp mới) ────────► Khởi tạo msg_count = 0, khởi tạo Semaphore
        │
        └──────► Kích thước > 0 (Đã có dữ liệu) ──► Tải lịch sử chat lên màn hình
                                                   Khởi tạo lại các Semaphore để tránh deadlock
```

---

## 4. Hướng dẫn Biên dịch và Chạy thử

### Cách chạy nhanh bằng Script
Chạy script tự động biên dịch và mở terminal:
```bash
bash restart.bash
```
*Script sẽ kiểm tra và thông báo nếu phát hiện file `chat_history.dat` cũ.*

### Cách chạy thủ công bằng Terminal
Mở hai cửa sổ terminal Linux độc lập:

1. **Biên dịch mã nguồn:**
   ```bash
   # Biên dịch server (cần -lpthread cho luồng và semaphores)
   gcc -o server server.c -lpthread
   
   # Biên dịch client
   gcc -o client client.c -lpthread
   ```

2. **Chạy Server:**
   ```bash
   ./server
   ```

3. **Chạy Client:**
   ```bash
   ./client
   ```

4. **Thử nghiệm tính năng lưu lịch sử (Persistent):**
   - **Bước 1:** Nhắn tin qua lại giữa Server và Client vài lượt.
   - **Bước 2:** Thoát cả hai tiến trình bằng cách gõ `/quit` hoặc nhấn `Ctrl+C`.
   - **Bước 3:** Chạy lại Server `./server` và Client `./client`. Bạn sẽ thấy toàn bộ lịch sử trò chuyện trước đó lập tức hiển thị lại trên màn hình trước khi bắt đầu phiên trò chuyện mới.

5. **Xóa lịch sử chat:**
   Nếu muốn bắt đầu một phòng chat hoàn toàn mới và xoá sạch lịch sử cũ, hãy xoá file dữ liệu vật lý trước khi chạy Server:
   ```bash
   rm -f chat_history.dat
   ```
