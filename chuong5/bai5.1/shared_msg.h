#ifndef SHARED_MSG_H
#define SHARED_MSG_H

#include <semaphore.h>

#define SHM_NAME "/shm_chat_5_1"
#define BUFFER_SIZE 1024

/* Cấu trúc cho một kênh truyền thông tin nhắn */
struct chat_channel {
    char message[BUFFER_SIZE];
    sem_t sem_filled;  // Báo hiệu tin nhắn mới đã được ghi (khởi tạo = 0)
    sem_t sem_empty;   // Báo hiệu bộ đệm đã trống và sẵn sàng ghi (khởi tạo = 1)
};

/* Cấu trúc toàn bộ bộ nhớ chia sẻ */
struct shared_data {
    struct chat_channel s2c;  // Server to Client channel
    struct chat_channel c2s;  // Client to Server channel
    volatile int server_alive; // Cờ báo Server đang chạy (1: chạy, 0: dừng)
    volatile int client_alive; // Cờ báo Client đang chạy (1: chạy, 0: dừng)
};

#endif /* SHARED_MSG_H */
