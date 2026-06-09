#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <sys/types.h>

#define SHM_NAME "/shm_data_5_2"
#define LOG_FILE "server.log"

/* Cấu trúc dữ liệu yêu cầu bởi đề bài */
struct data {
    char name[25];
    int tuoi;
};

/* Cấu trúc dữ liệu bộ nhớ chia sẻ */
struct shared_data {
    pid_t server_pid;          // PID của Server để Client gửi tín hiệu
    struct data user_data;     // Vùng chứa thông tin do Client ghi
    volatile int data_ready;   // 1: Client đã ghi xong dữ liệu
    volatile int processed;    // 1: Server đã đọc và ghi log xong
    volatile int server_alive; // 1: Server đang chạy
};

#endif /* SHARED_DATA_H */
