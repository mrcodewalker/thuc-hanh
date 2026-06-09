#ifndef SHARED_CHAT_H
#define SHARED_CHAT_H

#include <semaphore.h>

#define SHM_FILE "chat_history.dat"
#define MAX_MSG_LEN 256
#define MAX_MESSAGES 200

/* Cấu trúc cho một tin nhắn đơn */
struct chat_message {
    char sender[10];      // "Server" hoặc "Client"
    char text[MAX_MSG_LEN];
};

/* Cấu trúc toàn bộ cơ sở dữ liệu chat được mmap */
struct chat_db {
    struct chat_message messages[MAX_MESSAGES];
    int msg_count;             // Tổng số tin nhắn đã lưu
    volatile int server_alive; // Cờ báo Server đang chạy
    volatile int client_alive; // Cờ báo Client đang chạy
    
    sem_t mutex;          // Bảo vệ msg_count và mảng tin nhắn khỏi xung đột ghi đồng thời
    sem_t sem_server_new; // Server báo hiệu tin mới, Client chờ nhận
    sem_t sem_client_new; // Client báo hiệu tin mới, Server chờ nhận
};

#endif /* SHARED_CHAT_H */
