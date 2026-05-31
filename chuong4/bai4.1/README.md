# Bài 4.1 - Chat Client-Server (Localhost)

## Mục tiêu

Viết chương trình chat theo mô hình **client-server**, cả 2 chạy trên **cùng 1 máy tính** (localhost / 127.0.0.1).

---

## Socket trong lập trình nhân Linux

### Socket là gì?

**Socket** là một điểm cuối (endpoint) của kênh truyền thông 2 chiều giữa 2 tiến trình. Trong Linux, socket được xem như một **file descriptor** đặc biệt - mọi thao tác đọc/ghi đều giống như đọc/ghi file.

```
Tiến trình A  <───── socket ─────> Tiến trình B
   (Server)      (kênh truyền)       (Client)
```

### Tại sao socket quan trọng trong nhân Linux?

Trong Linux, "mọi thứ đều là file". Socket là cách hệ điều hành trừu tượng hóa giao tiếp mạng:
- Socket được cấp 1 **file descriptor** (số nguyên)
- Dùng `read()`/`write()` hoặc `recv()`/`send()` để truyền dữ liệu
- Nhân Linux quản lý buffer, TCP/IP stack, định tuyến gói tin

### Các loại Socket

| Loại | Hằng số | Giao thức | Đặc điểm |
|------|---------|-----------|----------|
| Stream | `SOCK_STREAM` | TCP | Tin cậy, có thứ tự, hướng kết nối |
| Datagram | `SOCK_DGRAM` | UDP | Nhanh, không đảm bảo, không kết nối |
| Raw | `SOCK_RAW` | IP | Truy cập trực tiếp tầng IP |

Bài này dùng **TCP (SOCK_STREAM)** vì chat cần dữ liệu đến đúng thứ tự, không mất.

### Address Family

| Hằng số | Ý nghĩa |
|---------|---------|
| `AF_INET` | IPv4 |
| `AF_INET6` | IPv6 |
| `AF_UNIX` | Giao tiếp giữa các tiến trình cùng máy (Unix domain socket) |

---

## Luồng hoạt động TCP Socket

```
     SERVER                          CLIENT
   ──────────                      ──────────
   socket()                        socket()
      │                               │
   bind()      ← gan dia chi/port     │
      │                               │
   listen()    ← bat dau lang nghe    │
      │                               │
   accept()    ← cho ket noi          │
      │  ◄───────────────────────  connect()
      │            (3-way handshake)   │
      │                               │
   recv()/send() ◄──────────────► send()/recv()
      │            (trao doi data)     │
      │                               │
   close()                         close()
```

### Giải thích từng hàm

| Hàm | Bên | Mô tả |
|-----|-----|--------|
| `socket()` | Cả 2 | Tạo socket, trả về file descriptor |
| `bind()` | Server | Gắn socket với IP + port cụ thể |
| `listen()` | Server | Chuyển socket sang chế độ lắng nghe |
| `accept()` | Server | Chấp nhận kết nối, trả về socket mới cho client |
| `connect()` | Client | Kết nối tới server |
| `send()` / `write()` | Cả 2 | Gửi dữ liệu |
| `recv()` / `read()` | Cả 2 | Nhận dữ liệu |
| `close()` | Cả 2 | Đóng kết nối |

---

## Cấu trúc dữ liệu quan trọng

### `struct sockaddr_in` (IPv4)

```c
struct sockaddr_in {
    short          sin_family;   // AF_INET
    unsigned short sin_port;     // Port (network byte order)
    struct in_addr sin_addr;     // Dia chi IP
    char           sin_zero[8];  // Padding
};
```

### Network Byte Order

Mạng dùng **big-endian**, máy tính có thể dùng little-endian. Phải chuyển đổi:

| Hàm | Ý nghĩa |
|-----|---------|
| `htons()` | Host TO Network Short (port) |
| `htonl()` | Host TO Network Long (IP) |
| `ntohs()` | Network TO Host Short |
| `ntohl()` | Network TO Host Long |

```c
server_addr.sin_port = htons(8888);              // Port
server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP
```

---

## Kiến trúc chương trình này

```
┌─────────────────────────────────────────────────────┐
│                  SERVER (server.c)                    │
├─────────────────────────────────────────────────────┤
│  Main thread:  gui tin nhan (fgets → send)           │
│  Recv thread:  nhan tin nhan (recv → printf)         │
└────────────────────┬────────────────────────────────┘
                     │ TCP socket
                     │ 127.0.0.1:8888
                     │
┌────────────────────┴────────────────────────────────┐
│                  CLIENT (client.c)                    │
├─────────────────────────────────────────────────────┤
│  Main thread:  gui tin nhan (fgets → send)           │
│  Recv thread:  nhan tin nhan (recv → printf)         │
└─────────────────────────────────────────────────────┘
```

**Tại sao dùng 2 thread?**
- Nếu chỉ 1 thread: chương trình bị **block** khi chờ nhập tin nhắn → không nhận được tin đến
- 1 thread gửi + 1 thread nhận → chat **2 chiều đồng thời** (full-duplex)

---

## Cách chạy

```bash
bash restart.bash
```

Script sẽ:
1. Kill process cũ (giải phóng port)
2. Compile cả `server.c` và `client.c`
3. Tự động mở 2 terminal (server + client) nếu có `gnome-terminal`/`xterm`/`konsole`

Nếu không tự mở được, chạy thủ công 2 terminal:
```bash
# Terminal 1
./server

# Terminal 2
./client
```

## Cách sử dụng

- Gõ tin nhắn rồi Enter để gửi
- Gõ `/quit` để thoát
- Cả server và client đều có thể gửi/nhận đồng thời

## Lưu ý kỹ thuật

1. **`SO_REUSEADDR`**: Cho phép tái sử dụng port ngay sau khi đóng (tránh lỗi "Address already in use")
2. **`127.0.0.1`**: Địa chỉ loopback - chỉ giao tiếp trong cùng máy
3. **Port 8888**: Port > 1024 không cần quyền root
4. Compile cần `-lpthread` vì dùng thread
5. `strcspn(buffer, "\n")` để xóa ký tự xuống dòng từ `fgets`
