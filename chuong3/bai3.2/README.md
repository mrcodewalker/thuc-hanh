# Bài 3.2 - Mutex File Write (Bảo vệ ghi file)

## Mục tiêu

Tạo 2 thread cùng ghi vào **chung 1 file**, sử dụng **mutex** để bảo vệ file và **wrapper function** thay cho hàm ghi file thông thường.

## Vấn đề nếu không dùng Mutex

Khi 2 thread cùng ghi file mà không đồng bộ:

```
Thread 1: fprintf(fp, "Hello from T1\n")
Thread 2: fprintf(fp, "Hello from T2\n")
```

Có thể xảy ra:
- Nội dung bị **xen kẽ** (interleaved): `"HelHello from T2\nlo from T1\n"`
- Dữ liệu bị **ghi đè** hoặc **mất**
- File bị **corrupt**

## Giải pháp: Wrapper Function + Mutex

Thay vì gọi `fprintf()` trực tiếp, ta tạo 1 hàm bọc (wrapper):

```c
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Wrapper function - thay thế fprintf trực tiếp */
void safe_write(const char *msg) {
    pthread_mutex_lock(&file_mutex);    // Khóa
    fprintf(fp, "%s", msg);             // Ghi an toàn
    fflush(fp);                         // Đảm bảo ghi xuống disk
    pthread_mutex_unlock(&file_mutex);  // Mở khóa
}
```

## Tại sao dùng Wrapper Function?

| Cách | Ưu điểm | Nhược điểm |
|------|----------|------------|
| Gọi `fprintf` trực tiếp | Đơn giản | Dễ quên lock/unlock, không tái sử dụng |
| **Wrapper function** | An toàn, tái sử dụng, dễ bảo trì | Thêm 1 lớp gọi hàm |

Wrapper function **đóng gói** logic mutex bên trong → người dùng chỉ cần gọi `safe_write()` mà không cần lo về đồng bộ.

## Luồng hoạt động

```
┌─────────────┐          ┌─────────────┐
│  Thread 1   │          │  Thread 2   │
└──────┬──────┘          └──────┬──────┘
       │                        │
       ├── safe_write() ────────┤
       │   ├─ lock()            │ (blocked, chờ)
       │   ├─ fprintf()         │
       │   ├─ fflush()          │
       │   └─ unlock() ────────→├── safe_write()
       │                        │   ├─ lock()
       │   (chờ)               │   ├─ fprintf()
       │                        │   ├─ fflush()
       │                        │   └─ unlock()
       ...                     ...
```

## Cách chạy

```bash
bash start.bash
```

## Kết quả mong đợi

- File `output.txt` chứa 10 dòng (5 từ Thread 1, 5 từ Thread 2)
- Mỗi dòng **nguyên vẹn**, không bị xen kẽ ký tự
- Chương trình in nội dung file ra màn hình để kiểm tra

## Giải thích code chi tiết

```c
FILE *fp = NULL;                         // File pointer chia sẻ
pthread_mutex_t file_mutex;              // Mutex bảo vệ file

/* Wrapper: đóng gói mutex + ghi file */
void safe_write(const char *msg) {
    pthread_mutex_lock(&file_mutex);
    if (fp != NULL) {                    // Kiểm tra file còn mở
        fprintf(fp, "%s", msg);
        fflush(fp);                      // Flush buffer ngay
    }
    pthread_mutex_unlock(&file_mutex);
}

void *writer_thread(void *arg) {
    int id = *(int *)arg;
    for (i = 1; i <= NUM_WRITES; i++) {
        snprintf(buffer, sizeof(buffer), 
                 "[Thread %d] Dong thu %d\n", id, i);
        safe_write(buffer);              // Gọi wrapper, không gọi fprintf
        usleep(150000);                  // 150ms delay
    }
}
```

## Lưu ý quan trọng

1. **`fflush(fp)`** - Đảm bảo dữ liệu được ghi xuống disk ngay, không nằm trong buffer
2. **Kiểm tra `fp != NULL`** - Phòng trường hợp file đã bị đóng
3. **`snprintf`** thay vì `sprintf` - Tránh buffer overflow
4. Wrapper function giúp **tách biệt** logic đồng bộ khỏi logic nghiệp vụ
5. Compile phải thêm flag `-lpthread`
