# Bài 3.7 - Tạo File Số Ngẫu Nhiên (Multi-thread)

## Mục tiêu

Tạo **10 file**, mỗi file chứa **5 triệu số ngẫu nhiên** có 1 chữ số (0-9). So sánh thời gian giữa:
- **Single thread**: tạo tuần tự từng file
- **Multi thread**: 10 thread tạo 10 file song song

## Thông số

| Tham số | Giá trị |
|---------|---------|
| Số file | 10 |
| Số/file | 5,000,000 |
| Tổng số | 50,000,000 |
| Kích thước/file | ~9.5 MB |
| Tổng dung lượng | ~95 MB |

## Kiến trúc

```
┌─────────────────────────────────────────────────────────┐
│                    SINGLE THREAD                          │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  File 0 → File 1 → File 2 → ... → File 9                │
│  ████████  ████████  ████████       ████████             │
│  (tuần tự, phải chờ file trước xong)                     │
│                                                           │
│  Tổng thời gian = T0 + T1 + T2 + ... + T9               │
│                                                           │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    MULTI THREAD (10 threads)              │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  Thread 0: File 0  ████████                              │
│  Thread 1: File 1  ████████                              │
│  Thread 2: File 2  ████████                              │
│  ...                                                      │
│  Thread 9: File 9  ████████                              │
│                                                           │
│  Tổng thời gian = max(T0, T1, ..., T9)                  │
│                                                           │
└─────────────────────────────────────────────────────────┘
```

## Kỹ thuật tối ưu I/O

### 1. Buffered Write (64KB buffer)

```c
char buffer[65536];  // 64KB buffer
int buf_pos = 0;

for (i = 0; i < 5000000; i++) {
    buffer[buf_pos++] = '0' + digit;
    buffer[buf_pos++] = '\n';

    if (buf_pos >= BUFFER_SIZE - 2) {
        fwrite(buffer, 1, buf_pos, fp);  // Ghi 1 lần lớn
        buf_pos = 0;
    }
}
```

**Tại sao?** Ghi 1 lần 64KB nhanh hơn ghi 5 triệu lần 2 bytes.

### 2. `rand_r()` thay vì `rand()`

```c
unsigned int local_seed = seed;
int digit = rand_r(&local_seed) % 10;
```

- `rand()`: dùng global state → cần lock khi multithread
- `rand_r()`: dùng local seed → thread-safe, không cần lock

### 3. Mỗi thread ghi file riêng

Không cần mutex vì mỗi thread thao tác trên file riêng biệt → **embarrassingly parallel**.

## Phân tích hiệu năng

### CPU-bound vs I/O-bound

| Loại | Đặc điểm | Multithread hiệu quả? |
|------|-----------|----------------------|
| CPU-bound | Tính toán nặng | Có (chia CPU) |
| **I/O-bound** | **Đọc/ghi disk** | **Phụ thuộc disk** |

Bài này là **I/O-bound**:
- SSD: multithread nhanh hơn đáng kể (SSD xử lý song song tốt)
- HDD: có thể chậm hơn (đầu đọc phải di chuyển giữa các file)

### Bottleneck

```
CPU: sinh số ngẫu nhiên → rất nhanh
I/O: ghi xuống disk → chậm (bottleneck)
```

→ Speedup phụ thuộc vào tốc độ disk, không phải CPU.

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- 10 file trong thư mục `data/` (random_00.txt đến random_09.txt)
- Mỗi file ~9.5 MB
- Bảng so sánh thời gian
- Speedup thường 2-5x trên SSD

## Cấu trúc file output

```
data/random_00.txt:
3
7
1
9
0
5
...
(5,000,000 dòng)
```

## Lưu ý

- Cần tạo thư mục `data/` trước (script tự tạo)
- `rand_r()` là thread-safe, mỗi thread có seed riêng
- Buffer 64KB giảm số lần system call `write()`
- Compile: `gcc -O2 -o random_files random_files.c -lpthread`
- Tổng dung lượng ~95 MB, đảm bảo đủ disk space
