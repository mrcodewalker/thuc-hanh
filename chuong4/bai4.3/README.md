# Bài 4.3 - Gửi/Nhận File giữa Nhà và Công ty

## Mục tiêu

Viết chương trình cho phép **gửi/nhận file** giữa 2 máy tính (ví dụ: máy ở nhà gửi file cho máy ở công ty) qua socket TCP.

## Mô hình

```
        "NHA"                           "CONG TY"
   ┌──────────────┐                ┌──────────────┐
   │ file_client  │                │ file_server  │
   │ (NGUOI GUI)  │ ─── file ────> │ (NGUOI NHAN) │
   │              │   qua TCP      │              │
   │ doc file     │                │ luu vao      │
   │ tu dia       │                │ received/    │
   └──────────────┘                └──────────────┘
```

- **Server** (công ty): lắng nghe, nhận file, lưu vào thư mục `received/`
- **Client** (nhà): đọc file từ đĩa, gửi qua mạng

## Vấn đề khi truyền file qua TCP

### 1. TCP là Stream, không phải Message

TCP gửi **dòng byte liên tục**, không phân biệt ranh giới message:

```
Gui:    [file.txt][1024][noi dung...]
Nhan:   co the den thanh nhieu manh: [file.t] [xt][10] [24][noi dun] [g...]
```

→ Cần **giao thức** (protocol) để biết: tên file dài bao nhiêu, file lớn bao nhiêu.

### 2. Giao thức tự định nghĩa

Chương trình dùng giao thức đơn giản:

```
┌─────────────┬──────────────┬─────────────┬──────────────┐
│  8 bytes    │  N bytes     │  8 bytes    │  M bytes     │
│  name_len   │  filename    │  file_size  │  file content│
│  (= N)      │              │  (= M)      │              │
└─────────────┴──────────────┴─────────────┴──────────────┘
```

Server đọc theo đúng thứ tự này → tách được tên file và nội dung.

### 3. recv() có thể trả về ít hơn yêu cầu

```c
// SAI: recv co the chi doc duoc 1 phan
recv(sock, buffer, 1000, 0);

// DUNG: lap den khi du n byte
ssize_t recv_all(int sock, void *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = recv(sock, p + total, n - total, 0);
        if (r <= 0) return r;
        total += r;
    }
    return total;
}
```

Đây là lý do phải có hàm `recv_all()` và `send_all()`.

## Network Byte Order cho số 64-bit

File size có thể > 4GB nên dùng `uint64_t`. Phải chuyển byte order:

```c
uint64_t file_size_be = htobe64(file_size);  // Host TO Big-Endian 64
send_all(sock, &file_size_be, sizeof(file_size_be));

// Ben nhan:
file_size = be64toh(file_size);  // Big-Endian TO Host 64
```

| Hàm | Ý nghĩa |
|-----|---------|
| `htobe64()` | Host to Big-Endian 64-bit |
| `be64toh()` | Big-Endian to Host 64-bit |

## Luồng hoạt động chi tiết

```
   CLIENT (Nha)                      SERVER (Cong ty)
   ────────────                      ────────────────
   fopen(file, "rb")                 socket() + bind() + listen()
   stat() → file_size                accept()  ← cho ket noi
   socket()                              │
   connect() ──────────────────────────>│
       │                                 │
   send name_len ──────────────────────>│ recv_all name_len
   send filename ──────────────────────>│ recv_all filename
   send file_size ─────────────────────>│ recv_all file_size
       │                                 │ fopen(received/file, "wb")
   loop:                                 loop:
     fread(buffer) ────────────────────>│  recv(buffer)
     send_all(buffer)                    │  fwrite(buffer)
       │  (hien thi % tien trinh)        │  (hien thi % tien trinh)
       │                                 │
   recv("OK") <─────────────────────────│ send("OK")
   close()                               close()
```

## Cách chạy

```bash
bash restart.bash
```

Script tạo sẵn `test_sample.txt` để demo.

### Máy NHẬN (Công ty)
```bash
./file_server
```
File nhận được lưu vào `received/`. Ghi nhớ IP hiển thị.

### Máy GỬI (Nhà)
```bash
./file_client <IP_CONG_TY> <file>
./file_client 192.168.1.10 baocao.pdf
```

### Test trên cùng 1 máy
```bash
# Terminal 1
./file_server
# Terminal 2
./file_client 127.0.0.1 test_sample.txt
```

## Kết quả mong đợi

- Client hiển thị tiến trình gửi (%)
- Server hiển thị tiến trình nhận (%)
- File xuất hiện trong `received/`
- Server gửi "OK" xác nhận, client báo thành công
- Có thể kiểm tra: `diff test_sample.txt received/test_sample.txt`

## Tính năng

| Tính năng | Mô tả |
|-----------|--------|
| Giữ tên file | Dùng `basename()` lấy tên, bỏ đường dẫn |
| File nhị phân | Mở mode `"rb"`/`"wb"` - gửi được mọi loại file (ảnh, pdf, zip) |
| File lớn | Dùng `uint64_t` cho kích thước > 4GB |
| Tiến trình | Hiển thị % real-time |
| Xác nhận | Server gửi ACK "OK" sau khi nhận xong |
| Nhiều file | Server vòng lặp, nhận nhiều file liên tiếp |

## Mở rộng từ bài này

- **2 chiều**: thêm chức năng download (công ty gửi về nhà)
- **Mã hóa**: dùng TLS/SSL để bảo mật khi qua Internet
- **Qua Internet**: cần port forwarding trên router hoặc VPN
- **Checksum**: thêm MD5/SHA để kiểm tra toàn vẹn file

## Lưu ý kỹ thuật

1. Mở file mode **binary** (`"rb"`/`"wb"`) để gửi đúng mọi loại file
2. **`send_all`/`recv_all`** đảm bảo gửi/nhận đủ byte
3. **`basename()`** cần header `<libgen.h>`
4. **`mkdir(RECV_DIR, 0755)`** tạo thư mục lưu file
5. Không cần `-lpthread` vì bài này không dùng thread
6. Để truyền qua Internet thật (nhà ↔ công ty khác mạng) cần cấu hình **port forwarding** hoặc **VPN**
