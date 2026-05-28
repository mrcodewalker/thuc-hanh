# Bài 3.6 - Đếm số lẻ từ 1 đến 100 tỷ

## Mục tiêu

Đếm số lượng số lẻ từ 1 đến 100,000,000,000 (100 tỷ). So sánh thời gian giữa:
- **Trường hợp 1**: Không dùng multithread (single thread)
- **Trường hợp 2**: Dùng multithread (8 threads)

## Phân tích bài toán

```
Số lẻ từ 1 đến N (N chẵn) = N / 2
→ Số lẻ từ 1 đến 100 tỷ = 50,000,000,000 (50 tỷ)
```

Tuy kết quả có thể tính trực tiếp bằng công thức, bài này minh họa cách **chia nhỏ công việc** cho nhiều thread.

## Chiến lược chia công việc

```
Tổng phạm vi: [1, 100,000,000,000]

Thread 0: [1,            12,500,000,000]
Thread 1: [12,500,000,001, 25,000,000,000]
Thread 2: [25,000,000,001, 37,500,000,000]
Thread 3: [37,500,000,001, 50,000,000,000]
Thread 4: [50,000,000,001, 62,500,000,000]
Thread 5: [62,500,000,001, 75,000,000,000]
Thread 6: [75,000,000,001, 87,500,000,000]
Thread 7: [87,500,000,001, 100,000,000,000]
```

Mỗi thread đếm số lẻ trong khoảng của mình → cộng lại.

## Cách đếm số lẻ trong khoảng [a, b]

```c
// Tìm số lẻ đầu tiên >= a
long long first_odd = (a % 2 == 1) ? a : a + 1;
// Tìm số lẻ cuối cùng <= b
long long last_odd = (b % 2 == 1) ? b : b - 1;
// Số lượng = (last - first) / 2 + 1
count = (last_odd - first_odd) / 2 + 1;
```

→ Không cần duyệt từng số, tính O(1) cho mỗi khoảng.

## Tại sao vẫn nhanh hơn khi dùng multithread?

Dù mỗi khoảng tính O(1), nhưng:
- **Single thread**: chia thành 10,000 đoạn nhỏ, duyệt tuần tự
- **Multi thread**: 8 thread chạy song song, mỗi thread xử lý 1 đoạn lớn

Với bài toán thực tế (duyệt từng số), multithread sẽ nhanh hơn **gấp nhiều lần**.

## Kiến trúc

```
┌─────────────────────────────────────────────────┐
│                    MAIN                           │
├─────────────────────────────────────────────────┤
│                                                   │
│  ┌─── Single Thread ───────────────────────┐     │
│  │ Dem tuan tu: [1..100B] chia 10M/doan    │     │
│  │ → Tong thoi gian: T1                    │     │
│  └─────────────────────────────────────────┘     │
│                                                   │
│  ┌─── Multi Thread (8 threads) ────────────┐     │
│  │ T0: [1..12.5B]     T4: [50B..62.5B]    │     │
│  │ T1: [12.5B..25B]   T5: [62.5B..75B]    │     │
│  │ T2: [25B..37.5B]   T6: [75B..87.5B]    │     │
│  │ T3: [37.5B..50B]   T7: [87.5B..100B]   │     │
│  │ → Tong thoi gian: T2                    │     │
│  └─────────────────────────────────────────┘     │
│                                                   │
│  Speedup = T1 / T2                               │
└─────────────────────────────────────────────────┘
```

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- Cả 2 cách đều cho kết quả: **50,000,000,000** số lẻ
- Multithread nhanh hơn (speedup phụ thuộc CPU)
- Bảng so sánh thời gian được in ra

## Đo thời gian chính xác

Sử dụng `clock_gettime(CLOCK_MONOTONIC)` thay vì `clock()`:
- `clock()`: đo CPU time (tổng tất cả thread) → không chính xác cho multithread
- `CLOCK_MONOTONIC`: đo wall-clock time (thời gian thực) → chính xác

```c
struct timespec t_start, t_end;
clock_gettime(CLOCK_MONOTONIC, &t_start);
// ... work ...
clock_gettime(CLOCK_MONOTONIC, &t_end);
double ms = (end.tv_sec - start.tv_sec) * 1000.0 +
            (end.tv_nsec - start.tv_nsec) / 1000000.0;
```

## Lưu ý

- Dùng `long long` vì 100 tỷ vượt quá `int` (max ~2.1 tỷ)
- Flag `-O2` giúp compiler tối ưu vòng lặp
- Số thread tối ưu thường = số core CPU
- Compile: `gcc -O2 -o count_odd count_odd.c -lpthread`
