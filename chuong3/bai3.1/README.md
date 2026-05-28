# Bài 3.1 - Mutex Counter (Đồng bộ biến đếm)

## Mục tiêu

Tạo 2 thread cùng tăng 1 biến đếm chung, sử dụng **mutex** để đảm bảo không xảy ra race condition.

## Vấn đề nếu không dùng Mutex

Khi 2 thread cùng truy cập 1 biến mà không đồng bộ:

```
Thread 1: đọc counter = 5
Thread 2: đọc counter = 5    ← cùng đọc giá trị cũ!
Thread 1: ghi counter = 6
Thread 2: ghi counter = 6    ← mất 1 lần tăng!
```

Kết quả cuối sẽ **nhỏ hơn** giá trị mong đợi → sai logic.

## Giải pháp: Mutex

Mutex (Mutual Exclusion) đảm bảo **chỉ 1 thread** được truy cập vùng critical section tại 1 thời điểm.

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Trong mỗi thread:
pthread_mutex_lock(&mutex);    // Khóa → thread khác phải chờ
counter++;                      // Critical section (an toàn)
pthread_mutex_unlock(&mutex);  // Mở khóa → thread khác được vào
```

## Luồng hoạt động

```
┌─────────────┐     ┌─────────────┐
│  Thread 1   │     │  Thread 2   │
└──────┬──────┘     └──────┬──────┘
       │                    │
       ├── lock() ──────────┤ (chờ)
       │   counter++        │
       ├── unlock() ────────┤
       │                    ├── lock()
       │   (chờ)           │   counter++
       │                    ├── unlock()
       ├── lock() ──────────┤
       │   counter++        │
       ...                 ...
```

## Các hàm pthread quan trọng

| Hàm | Mô tả |
|-----|--------|
| `pthread_mutex_init()` | Khởi tạo mutex |
| `pthread_mutex_lock()` | Khóa mutex (block nếu đã bị khóa) |
| `pthread_mutex_unlock()` | Mở khóa mutex |
| `pthread_mutex_destroy()` | Hủy mutex khi không dùng nữa |
| `pthread_create()` | Tạo thread mới |
| `pthread_join()` | Chờ thread kết thúc |

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- Mỗi thread tăng counter 10 lần
- Tổng cộng: counter = **20**
- Nếu kết quả = 20 → mutex hoạt động đúng ✓

## Giải thích code

```c
int counter = 0;                    // Biến chia sẻ giữa 2 thread
pthread_mutex_t mutex;              // Mutex bảo vệ biến counter

void *thread_func(void *arg) {
    for (i = 0; i < MAX_COUNT; i++) {
        pthread_mutex_lock(&mutex);   // ← Vào critical section
        counter++;                     // ← Thao tác an toàn
        pthread_mutex_unlock(&mutex); // ← Rời critical section
        usleep(100000);               // Delay 100ms để thấy rõ
    }
}
```

## Lưu ý

- `PTHREAD_MUTEX_INITIALIZER` là cách khởi tạo tĩnh (static), không cần gọi `pthread_mutex_init()`
- Luôn `unlock` sau khi `lock`, nếu quên sẽ gây **deadlock**
- Compile phải thêm flag `-lpthread`
