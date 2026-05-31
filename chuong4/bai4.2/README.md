# Bài 4.2 - Chat Client-Server qua mạng LAN

## Mục tiêu

Viết chương trình chat client-server cho phép chat trong **mạng LAN** - nhiều người dùng trên các máy tính khác nhau cùng chat với nhau (multi-client).

## Điểm khác biệt so với Bài 4.1

| Đặc điểm | Bài 4.1 (Localhost) | Bài 4.2 (LAN) |
|----------|---------------------|---------------|
| Địa chỉ bind | `127.0.0.1` | `INADDR_ANY` (0.0.0.0) |
| Số client | 1 | Nhiều (tối đa 10) |
| Phạm vi | Cùng 1 máy | Nhiều máy trong LAN |
| Broadcast | Không | Có (gửi tới mọi người) |
| Thread/client | Không cần | 1 thread / client |

## Khái niệm mạng LAN

### INADDR_ANY (0.0.0.0)

```c
server_addr.sin_addr.s_addr = INADDR_ANY;
```

- `127.0.0.1`: chỉ chấp nhận kết nối từ **chính máy đó**
- `INADDR_ANY` (0.0.0.0): chấp nhận kết nối từ **mọi network interface** (WiFi, Ethernet, loopback) → cho phép máy khác trong LAN kết nối

### Địa chỉ IP trong LAN

```
Dải IP private (LAN):
  192.168.x.x   (phổ biến nhất - router gia đình)
  10.x.x.x      (mạng công ty lớn)
  172.16-31.x.x (mạng vừa)
```

Xem IP máy mình:
```bash
hostname -I        # Cách nhanh
ip addr            # Chi tiết hơn
ifconfig           # Cách cũ
```

## Kiến trúc Multi-Client

```
                    ┌─────────────────────────┐
                    │      SERVER             │
                    │   (0.0.0.0:9000)        │
                    │                         │
                    │  ┌──────────────────┐   │
                    │  │ accept() loop    │   │
                    │  └────────┬─────────┘   │
                    │           │ tao thread  │
                    │  ┌────────┴─────────┐   │
                    │  │ Thread/client    │   │
                    │  │ + broadcast()    │   │
                    │  └──────────────────┘   │
                    └───┬──────┬──────┬───────┘
                        │      │      │
            ┌───────────┘      │      └───────────┐
            │                  │                  │
       ┌────┴────┐        ┌────┴────┐        ┌────┴────┐
       │ Client  │        │ Client  │        │ Client  │
       │ Alice   │        │  Bob    │        │ Carol   │
       │(may A)  │        │ (may B) │        │ (may C) │
       └─────────┘        └─────────┘        └─────────┘
```

## Cơ chế Broadcast

Khi 1 client gửi tin, server **chuyển tiếp** tới tất cả client khác:

```c
void broadcast(const char *msg, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_sock) {  // Tru nguoi gui
            send(client_sockets[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
```

**Mutex** bảo vệ mảng `client_sockets[]` vì nhiều thread cùng truy cập.

## Mô hình Thread-per-Client

```c
while (1) {
    new_sock = accept(...);              // Cho client moi
    pthread_create(&tid, NULL,
                   handle_client, new_sock);  // Tao thread rieng
    pthread_detach(tid);                 // Tu giai phong khi xong
}
```

- Mỗi client có 1 thread riêng xử lý → các client độc lập
- `pthread_detach()`: thread tự dọn dẹp khi kết thúc, không cần `join`

## Cách chạy

```bash
bash restart.bash
```

### Trên máy làm Server
```bash
./server
```
Ghi nhớ IP hiển thị (vd: 192.168.1.10)

### Trên máy Client (trong cùng LAN)
```bash
./client <IP_SERVER> <TEN>
./client 192.168.1.10 Alice
```

### Test trên cùng 1 máy
```bash
# Terminal 1
./server
# Terminal 2
./client 127.0.0.1 Bob
# Terminal 3
./client 127.0.0.1 Carol
```

## Cách sử dụng

- Gõ tin nhắn + Enter để gửi cho mọi người
- Khi có người vào/ra, mọi người nhận thông báo
- Gõ `/quit` để rời phòng

## Xử lý sự cố

| Lỗi | Nguyên nhân | Giải pháp |
|-----|-------------|-----------|
| `connect: Connection refused` | Server chưa chạy | Chạy `./server` trước |
| `connect: No route to host` | Firewall chặn | Mở port 9000, kiểm tra cùng LAN |
| `bind: Address already in use` | Port đang dùng | Đợi vài giây hoặc đổi port |
| Client khác không kết nối được | Firewall | `sudo ufw allow 9000` |

## Lưu ý kỹ thuật

1. **`INADDR_ANY`** thay vì IP cụ thể để nhận kết nối từ LAN
2. **`pthread_detach`** tránh memory leak khi nhiều client vào/ra
3. **`inet_ntoa()`** chuyển IP từ dạng số sang chuỗi để in log
4. **`inet_pton()`** (client) chuyển chuỗi IP sang dạng nhị phân, an toàn hơn `inet_addr`
5. Cần đảm bảo **firewall** cho phép port 9000
6. Compile cần `-lpthread`
