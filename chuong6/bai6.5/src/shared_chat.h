#ifndef SHARED_CHAT_H
#define SHARED_CHAT_H

#include <semaphore.h>

#define BUFFER_SIZE 1024

/* Cấu trúc cho một kênh truyền tin nhắn */
struct chat_channel {
    char message[BUFFER_SIZE];
    sem_t sem_filled;  // Báo hiệu tin mới (khởi tạo = 0)
    sem_t sem_empty;   // Báo hiệu đã đọc, sẵn sàng ghi tiếp (khởi tạo = 1)
};

/* Cấu trúc toàn bộ bộ nhớ chia sẻ */
struct shared_data {
    struct chat_channel s2c;   // Kênh Server to Client
    struct chat_channel c2s;   // Kênh Client to Server
    volatile int server_alive; // Cờ báo Server đang chạy
    volatile int client_alive; // Cờ báo Client đang chạy
};

#endif /* SHARED_CHAT_H */
