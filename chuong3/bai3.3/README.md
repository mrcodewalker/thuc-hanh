# Bài 3.3 - Mutex Combined (Counter + File)

## Mục tiêu

Kết hợp cả 2 bài trước: 2 thread vừa **tăng biến đếm** vừa **ghi kết quả vào file**, sử dụng **2 mutex riêng biệt** và **wrapper functions** cho cả 2 thao tác.

## Kiến trúc chương trình

```
┌──────────────────────────────────────────────────┐
│                    MAIN                            │
├──────────────────────────────────────────────────┤
│                                                    │
│  ┌────────────┐              ┌────────────┐       │
│  │  Thread 1  │              │  Thread 2  │       │
│  └─────┬──────┘              └─────┬──────┘       │
│        │                           │               │
│        ▼                           ▼               │
│  ┌──────────────────────────────────────────┐     │
│  │         safe_increment()                  │     │
│  │         [counter_mutex]                   │     │
│  │         counter++                         │     │
│  └──────────────────────────────────────────┘     │
│        │                           │               │
│        ▼                           ▼               │
│  ┌──────────────────────────────────────────┐     │
│  │         safe_log()                        │     │
│  │         [file_mutex]                      │     │
│  │         fprintf(fp, ...)                  │     │
│  └──────────────────────────────────────────┘     │
│                                                    │
└──────────────────────────────────────────────────┘
```

## Tại sao dùng 2 Mutex riêng biệt?

| Cách | Ưu/Nhược |
|------|-----------|
| 1 mutex cho cả 2 | Đơn giản nhưng **chậm** - thread phải chờ cả khi chỉ cần ghi file |
| **2 mutex riêng** | Nhanh hơn - thread có thể tăng counter trong khi thread khác ghi file |

```c
pthread_mutex_t counter_mutex;  // Chỉ bảo vệ biến counter
pthread_mutex_t file_mutex;     // Chỉ bảo vệ file pointer
```

→ **Fine-grained locking**: khóa càng nhỏ, hiệu năng càng cao.

## 2 Wrapper Functions

### 1. `safe_increment()` - Tăng counter an toàn

```c
int safe_increment(int thread_id) {
    int val;
    pthread_mutex_lock(&counter_mutex);
    counter++;
    val = counter;
    pthread_mutex_unlock(&counter_mutex);
    return val;  // Trả về giá trị mới
}
```

### 2. `safe_log()` - Ghi file an toàn

```c
void safe_log(const char *msg) {
    pthread_mutex_lock(&file_mutex);
    if (fp != NULL) {
        fprintf(fp, "%s", msg);
        fflush(fp);
    }
    pthread_mutex_unlock(&file_mutex);
}
```

## Luồng hoạt động chi tiết

```
Thread 1                          Thread 2
────────                          ────────
safe_increment()                  (chờ counter_mutex)
  lock(counter_mutex)
  counter: 0 → 1
  unlock(counter_mutex)
                                  safe_increment()
                                    lock(counter_mutex)
                                    counter: 1 → 2
                                    unlock(counter_mutex)

safe_log("counter=1")            safe_log("counter=2")
  lock(file_mutex)                 (chờ file_mutex)
  fprintf(...)
  unlock(file_mutex)
                                    lock(file_mutex)
                                    fprintf(...)
                                    unlock(file_mutex)
```

**Lưu ý**: 2 mutex độc lập nên Thread 2 có thể `safe_increment()` trong khi Thread 1 đang `safe_log()` → tăng hiệu năng!

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- Mỗi thread tăng counter 8 lần → tổng = **16**
- File `result.txt` ghi lại toàn bộ quá trình
- Cuối chương trình đọc lại file và in ra màn hình

## So sánh 3 bài

| Bài | Tài nguyên chia sẻ | Số mutex | Wrapper |
|-----|-------------------|----------|---------|
| 3.1 | Biến counter | 1 | Không |
| 3.2 | File pointer | 1 | `safe_write()` |
| **3.3** | **Counter + File** | **2** | **`safe_increment()` + `safe_log()`** |

## Khái niệm nâng cao

### Fine-grained Locking
- Dùng nhiều mutex nhỏ thay vì 1 mutex lớn
- Giảm thời gian chờ (contention)
- Trade-off: code phức tạp hơn, cần cẩn thận tránh deadlock

### Deadlock Prevention
Khi dùng nhiều mutex, cần tránh deadlock:
```
Thread 1: lock(A) → lock(B)    ← OK
Thread 2: lock(A) → lock(B)    ← OK (cùng thứ tự)

Thread 1: lock(A) → lock(B)    ← NGUY HIỂM!
Thread 2: lock(B) → lock(A)    ← Deadlock!
```

**Quy tắc**: Luôn lock các mutex theo **cùng 1 thứ tự** trong tất cả thread.

Trong bài này, mỗi wrapper chỉ dùng 1 mutex nên không có nguy cơ deadlock.

## Lưu ý

- Compile cần flag `-lpthread`
- `usleep()` giúp thấy rõ sự xen kẽ giữa 2 thread
- Kết quả cuối luôn đúng nhờ mutex, bất kể thứ tự chạy
