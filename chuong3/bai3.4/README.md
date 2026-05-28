# Bài 3.4 - Semaphore File Sync

## Mục tiêu

Tạo 3 thread, các thread lần lượt tăng 1 biến chung thêm 1 đơn vị và ghi giá trị mới vào **1 trong 2 file output**. Sử dụng **semaphore** để đồng bộ việc ghi vào 2 file.

## Semaphore là gì?

Semaphore là cơ chế đồng bộ dùng **biến đếm** (counter):
- `sem_wait()` (P operation): giảm counter đi 1. Nếu counter = 0 → **block** thread
- `sem_post()` (V operation): tăng counter lên 1. Nếu có thread đang chờ → **đánh thức**

```
Semaphore(1) = Binary Semaphore ≈ Mutex
Semaphore(N) = Counting Semaphore (cho phép N thread cùng vào)
```

## So sánh Mutex vs Semaphore

| Đặc điểm | Mutex | Semaphore |
|-----------|-------|-----------|
| Giá trị | 0 hoặc 1 | 0 đến N |
| Ownership | Thread lock phải unlock | Bất kỳ thread nào cũng post được |
| Mục đích | Bảo vệ critical section | Đồng bộ + giới hạn truy cập |
| Dùng khi | 1 tài nguyên, 1 thread | Nhiều tài nguyên, nhiều thread |

## Kiến trúc chương trình

```
┌──────────────────────────────────────────────────────┐
│                      MAIN                             │
├──────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ Thread 1 │  │ Thread 2 │  │ Thread 3 │            │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘            │
│       │              │              │                  │
│       ▼              ▼              ▼                  │
│  ┌────────────────────────────────────────┐           │
│  │  counter++ (mutex bao ve)              │           │
│  └────────────────────┬───────────────────┘           │
│                       │                                │
│            ┌──────────┴──────────┐                    │
│            │                     │                    │
│     Gia tri chan            Gia tri le                 │
│            │                     │                    │
│            ▼                     ▼                    │
│  ┌─────────────────┐  ┌─────────────────┐            │
│  │  sem_file1       │  │  sem_file2       │            │
│  │  → output1.txt   │  │  → output2.txt   │            │
│  └─────────────────┘  └─────────────────┘            │
│                                                        │
└──────────────────────────────────────────────────────┘
```

## Luồng hoạt động

```
Thread 1              Thread 2              Thread 3
────────              ────────              ────────
lock(mutex)
counter: 0→1
unlock(mutex)
                      lock(mutex)
                      counter: 1→2
                      unlock(mutex)
                                            lock(mutex)
                                            counter: 2→3
                                            unlock(mutex)

1 là lẻ → file2      2 là chẵn → file1     3 là lẻ → file2
sem_wait(sem2)        sem_wait(sem1)        sem_wait(sem2) ← BLOCK
ghi file2             ghi file1             (chờ Thread 1 xong)
sem_post(sem2)        sem_post(sem1)        ghi file2
                                            sem_post(sem2)
```

## Các hàm Semaphore

| Hàm | Mô tả |
|-----|--------|
| `sem_init(&sem, 0, val)` | Khởi tạo semaphore với giá trị `val` |
| `sem_wait(&sem)` | P() - Giảm 1, block nếu = 0 |
| `sem_post(&sem)` | V() - Tăng 1, đánh thức thread chờ |
| `sem_destroy(&sem)` | Hủy semaphore |

Tham số thứ 2 của `sem_init`: 0 = chia sẻ giữa threads, 1 = chia sẻ giữa processes.

## Logic phân chia file

```c
if (val % 2 == 0)
    write_file1(id, val);  // Số chẵn → output1.txt
else
    write_file2(id, val);  // Số lẻ → output2.txt
```

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- 3 threads × 6 lần = counter cuối = **18**
- `output1.txt`: chứa các giá trị chẵn (2, 4, 6, 8, ...)
- `output2.txt`: chứa các giá trị lẻ (1, 3, 5, 7, ...)
- Không có dòng nào bị xen kẽ hay corrupt

## Tại sao dùng Semaphore thay vì Mutex?

Trong bài này dùng binary semaphore (giá trị 1) nên tương đương mutex. Nhưng semaphore linh hoạt hơn:

```c
// Nếu muốn cho 2 thread cùng ghi 1 file (buffered write):
sem_init(&sem_file1, 0, 2);  // Counting semaphore = 2
```

→ Semaphore cho phép **mở rộng** số lượng truy cập đồng thời mà không cần thay đổi logic.

## Lưu ý

- Header `<semaphore.h>` cần cho semaphore
- Compile: `gcc -o prog prog.c -lpthread`
- `sem_init` với tham số 0 = thread-shared (không phải process-shared)
- Luôn `sem_destroy` khi kết thúc
