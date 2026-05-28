# Bài 3.5 - Matrix Multiplication (Multi-thread)

## Mục tiêu

Tạo 10 ma trận kích thước **200×200**, nhân 10 ma trận đó với nhau sử dụng **multi-thread** để tăng tốc.

## Chiến lược nhân ma trận

Nhân tuần tự: `Result = A1 × A2 × A3 × ... × A10`

Mỗi phép nhân 2 ma trận được **chia cho nhiều thread** xử lý song song.

```
Bước 1: R = A1 × A2        (4 threads song song)
Bước 2: R = R × A3         (4 threads song song)
Bước 3: R = R × A4         (4 threads song song)
...
Bước 9: R = R × A10        (4 threads song song)
```

## Cách chia công việc cho Thread

Mỗi phép nhân C = A × B (200×200):
- Chia 200 hàng cho 4 thread
- Thread 0: tính hàng 0-49
- Thread 1: tính hàng 50-99
- Thread 2: tính hàng 100-149
- Thread 3: tính hàng 150-199

```
┌─────────────────────────────────────────┐
│              Ma tran C (200x200)         │
├─────────────────────────────────────────┤
│  Thread 0: hang 0-49     ████████████   │
├─────────────────────────────────────────┤
│  Thread 1: hang 50-99    ████████████   │
├─────────────────────────────────────────┤
│  Thread 2: hang 100-149  ████████████   │
├─────────────────────────────────────────┤
│  Thread 3: hang 150-199  ████████████   │
└─────────────────────────────────────────┘
```

## Tại sao chia theo hàng?

Khi tính `C[i][j] = Σ A[i][k] * B[k][j]`:
- Mỗi hàng `i` của C **độc lập** với các hàng khác
- Không cần mutex hay đồng bộ giữa các thread!
- Đây là **embarrassingly parallel** - dễ song song hóa

## Cấu trúc dữ liệu truyền cho Thread

```c
typedef struct {
    Matrix *A;        // Con trỏ ma trận A
    Matrix *B;        // Con trỏ ma trận B
    Matrix *C;        // Con trỏ ma trận kết quả
    int start_row;    // Hàng bắt đầu (inclusive)
    int end_row;      // Hàng kết thúc (exclusive)
} ThreadData;
```

## Hàm nhân song song

```c
void *multiply_rows(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (i = data->start_row; i < data->end_row; i++) {
        for (j = 0; j < SIZE; j++) {
            double sum = 0.0;
            for (k = 0; k < SIZE; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}
```

## Phân tích hiệu năng

| Cách | Thời gian (ước tính) | Ghi chú |
|------|----------------------|---------|
| 1 thread | ~X giây | Tuần tự |
| 4 threads | ~X/3.5 giây | Gần tuyến tính (overhead nhỏ) |
| 8 threads | ~X/6 giây | Phụ thuộc số core CPU |

**Speedup** phụ thuộc vào:
- Số core vật lý của CPU
- Cache locality (ma trận 200×200 vừa L2 cache)
- Overhead tạo/join thread

## Bộ nhớ sử dụng

```
Mỗi ma trận: 200 × 200 × 8 bytes (double) = 320 KB
10 ma trận + 2 buffer = 12 × 320 KB ≈ 3.75 MB
```

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- In thời gian thực hiện 9 phép nhân
- In góc 5×5 của ma trận đầu và kết quả
- Thông tin bộ nhớ sử dụng

## Tối ưu hóa đã áp dụng

1. **`-O2` flag**: Compiler optimization (loop unrolling, vectorization)
2. **Chia theo hàng**: Tận dụng cache locality (truy cập liên tiếp trong bộ nhớ)
3. **Giá trị nhỏ (0.0-0.9)**: Tránh overflow khi nhân nhiều lần
4. **`memcpy` cho copy**: Nhanh hơn copy từng phần tử

## Mở rộng

- Tăng `NUM_THREADS` để test với nhiều thread hơn
- Thay đổi `SIZE` để test với ma trận lớn hơn
- So sánh thời gian 1 thread vs N threads để thấy speedup

## Lưu ý

- Compile cần `-lpthread` và `-lm`
- Flag `-O2` giúp tăng tốc đáng kể
- Ma trận dùng `double` cho độ chính xác
- `srand(42)` cho kết quả reproducible
