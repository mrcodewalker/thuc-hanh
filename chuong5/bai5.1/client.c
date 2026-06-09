/*
 * Bai 5.1: Chat Client-Server tren cung 1 may (localhost) bang Shared Memory
 * CLIENT - Ket noi toi bo nho chia se da khoi tao boi Server
 *
 * Su dung 2 thread:
 *  - 1 thread nhan tin nhan tu server qua shared memory (dung Semaphore dong bo)
 *  - thread chinh nhap lieu tu terminal (dung select de khong bi block) va gui qua shared memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include "shared_msg.h"

struct shared_data *shared = NULL;
int shm_fd = -1;
volatile int running = 1;
pthread_t recv_thread;

/* Hàm dọn dẹp tài nguyên bên Client */
void cleanup() {
    printf("\n[Client] Dang dong ket noi...\n");
    running = 0;

    // Huy thread nhan tin
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    if (shared != NULL) {
        shared->client_alive = 0;
        
        // Post semaphore phia gui de danh thuc Server neu Server dang block doi Client
        sem_post(&shared->c2s.sem_filled);
        
        munmap(shared, sizeof(struct shared_data));
    }

    if (shm_fd != -1) {
        close(shm_fd);
    }
    printf("[Client] Da ngat ket noi. Tam biet!\n");
}

/* Bộ xử lý tín hiệu (Ctrl+C, SIGTERM) */
void signal_handler(int sig) {
    running = 0;
    if (shared != NULL) {
        shared->client_alive = 0;
        // Báo thức các thread đang đợi
        sem_post(&shared->s2c.sem_filled);
        sem_post(&shared->s2c.sem_empty);
        sem_post(&shared->c2s.sem_filled);
        sem_post(&shared->c2s.sem_empty);
    }
}

/* Thread nhận tin nhắn từ Server */
void *receive_handler(void *arg) {
    while (running) {
        // Chờ Server ghi tin nhắn vào s2c.message
        if (sem_wait(&shared->s2c.sem_filled) == -1) {
            if (errno == EINTR) {
                if (!running) break;
                continue;
            }
            break;
        }

        if (!running) break;

        // Kiểm tra nếu Server đã tắt hoặc ngắt kết nối
        if (!shared->server_alive) {
            printf("\n[!] Server da ngat ket noi.\n");
            running = 0;
            break;
        }

        // Nếu Server gửi lệnh thoát
        if (strcmp(shared->s2c.message, "/quit") == 0) {
            printf("\n[!] Server da thoat.\n");
            running = 0;
            break;
        }

        // In tin nhắn từ Server
        printf("\r[Server]: %s\n[Client]: ", shared->s2c.message);
        fflush(stdout);

        // Báo cho Server biết đã đọc xong
        sem_post(&shared->s2c.sem_empty);
    }
    return NULL;
}

int main() {
    char buffer[BUFFER_SIZE];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.1 - CHAT CLIENT (Shared Memory)      ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Đăng ký tín hiệu
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 1. Mở phân vùng bộ nhớ chia sẻ đã được Server tạo
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[ERROR] connect (server da chay chua?)");
        return 1;
    }

    // 2. Ánh xạ (Map) bộ nhớ chia sẻ
    shared = (struct shared_data *)mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(shm_fd);
        return 1;
    }

    // 3. Kiểm tra xem Server có thực sự đang chạy không
    if (!shared->server_alive) {
        printf("[ERROR] Server chua san sang hoac da tat.\n");
        munmap(shared, sizeof(struct shared_data));
        close(shm_fd);
        return 1;
    }

    // 4. Kết nối thành công, đặt cờ client_alive = 1
    shared->client_alive = 1;
    printf("[Client] ✓ Da ket noi den server!\n");
    printf("[Client] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 5. Tạo thread nhận tin nhắn từ Server
    if (pthread_create(&recv_thread, NULL, receive_handler, NULL) != 0) {
        perror("[ERROR] pthread_create");
        cleanup();
        return 1;
    }

    // 6. Vòng lặp gửi tin nhắn
    while (running) {
        printf("[Client]: ");
        fflush(stdout);

        // Sử dụng select để kiểm tra đầu vào stdin phi chặn
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret == -1) {
            if (errno == EINTR) {
                if (!running) break;
                continue;
            }
            perror("select");
            break;
        }

        if (!running) break;

        // Kiểm tra xem Server có chết đột ngột không
        if (!shared->server_alive) {
            printf("\n[!] Server da ngat ket noi.\n");
            running = 0;
            break;
        }

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                // Trường hợp Ctrl+D
                running = 0;
                break;
            }

            if (!running) break;

            // Xóa ký tự xuống dòng ở cuối
            buffer[strcspn(buffer, "\n")] = '\0';

            // Chờ cho đến khi kênh Client->Server trống
            if (sem_wait(&shared->c2s.sem_empty) == -1) {
                if (errno == EINTR) {
                    if (!running) break;
                    continue;
                }
                break;
            }

            if (!running) break;

            // Sao chép tin nhắn vào shared memory
            strncpy(shared->c2s.message, buffer, BUFFER_SIZE - 1);
            shared->c2s.message[BUFFER_SIZE - 1] = '\0';

            // Báo cho Server biết tin nhắn đã ghi xong
            sem_post(&shared->c2s.sem_filled);

            // Kiểm tra lệnh thoát
            if (strcmp(buffer, "/quit") == 0) {
                running = 0;
                break;
            }
        }
    }

    cleanup();
    return 0;
}
