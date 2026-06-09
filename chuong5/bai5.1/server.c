/*
 * Bai 5.1: Chat Client-Server tren cung 1 may (localhost) bang Shared Memory
 * SERVER - Khoi tao va quan ly bo nho chia se (POSIX Shared Memory)
 *
 * Su dung 2 thread:
 *  - 1 thread nhan tin nhan tu client qua shared memory (dung Semaphore dong bo)
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

/* Hàm giải phóng tài nguyên khi thoát */
void cleanup() {
    printf("\n[Server] Dang thu hoi tai nguyen...\n");
    running = 0;

    // Huy thread nhan tin
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    if (shared != NULL) {
        shared->server_alive = 0;
        // Giai phong semaphore
        sem_destroy(&shared->s2c.sem_filled);
        sem_destroy(&shared->s2c.sem_empty);
        sem_destroy(&shared->c2s.sem_filled);
        sem_destroy(&shared->c2s.sem_empty);
        
        munmap(shared, sizeof(struct shared_data));
    }

    if (shm_fd != -1) {
        close(shm_fd);
    }
    
    // Xoa phan vung bo nho chia se khoi he thong
    shm_unlink(SHM_NAME);
    printf("[Server] Da xoa Shared Memory. Tam biet!\n");
}

/* Bo xu ly tin hieu (Ctrl+C, SIGTERM) */
void signal_handler(int sig) {
    running = 0;
    if (shared != NULL) {
        shared->server_alive = 0;
        // Post tat ca semaphore de danh thuc cac thread dang block neu co
        sem_post(&shared->s2c.sem_filled);
        sem_post(&shared->s2c.sem_empty);
        sem_post(&shared->c2s.sem_filled);
        sem_post(&shared->c2s.sem_empty);
    }
}

/* Thread nhan tin nhan tu client */
void *receive_handler(void *arg) {
    while (running) {
        // Cho cho den khi client ghi tin nhan vao c2s.message
        if (sem_wait(&shared->c2s.sem_filled) == -1) {
            if (errno == EINTR) {
                if (!running) break;
                continue;
            }
            break;
        }

        if (!running) break;

        // Kiem tra neu client da ngat ket noi
        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }

        // Neu client gui lenh thoat
        if (strcmp(shared->c2s.message, "/quit") == 0) {
            printf("\n[!] Client da thoat.\n");
            running = 0;
            break;
        }

        // In tin nhan nhan duoc tu Client
        printf("\r[Client]: %s\n[Server]: ", shared->c2s.message);
        fflush(stdout);

        // Bao cho client biet da doc xong de client co the gui tiep
        sem_post(&shared->c2s.sem_empty);
    }
    return NULL;
}

int main() {
    char buffer[BUFFER_SIZE];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.1 - CHAT SERVER (Shared Memory)      ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Dang ky tin hieu de don dep khi bi Ctrl+C
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 1. Tao phan vung bo nho chia se
    // Xoa phan vung cu neu no van con ton tai tu lan chay truoc lam loi
    shm_unlink(SHM_NAME);
    
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[ERROR] shm_open");
        return 1;
    }

    // 2. Thiet lap kich thuoc phan vung bo nho chia se
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
        perror("[ERROR] ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // 3. Anh xa (Map) phan vung vao khong gian dia chi cua tien trinh
    shared = (struct shared_data *)mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // 4. Khoi tao cac gia tri trong shared memory
    shared->server_alive = 1;
    shared->client_alive = 0;

    // Khoi tao cac semaphore chia se giua cac tien trinh (pshared = 1)
    if (sem_init(&shared->s2c.sem_filled, 1, 0) == -1 ||
        sem_init(&shared->s2c.sem_empty, 1, 1) == -1 ||
        sem_init(&shared->c2s.sem_filled, 1, 0) == -1 ||
        sem_init(&shared->c2s.sem_empty, 1, 1) == -1) {
        perror("[ERROR] sem_init");
        munmap(shared, sizeof(struct shared_data));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    printf("[Server] Khoi tao Shared Memory thanh cong.\n");
    printf("[Server] Dang cho client ket noi...\n\n");

    // 5. Cho Client ket noi
    while (running && !shared->client_alive) {
        usleep(100000); // Cho 100ms moi lan kiem tra
    }

    if (!running) {
        cleanup();
        return 0;
    }

    printf("[Server] ✓ Client da ket noi!\n");
    printf("[Server] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 6. Tao thread de nhan tin nhan tu Client
    if (pthread_create(&recv_thread, NULL, receive_handler, NULL) != 0) {
        perror("[ERROR] pthread_create");
        cleanup();
        return 1;
    }

    // 7. Vong lap nhap/gui tin nhan
    while (running) {
        printf("[Server]: ");
        fflush(stdout);

        // Su dung select de kiem tra stdin co du lieu chua (giup vong lap khong bi block cuu canh khi client thoat)
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

        // Kiem tra xem client co thoat dot ngot khong
        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                // Truong hop Ctrl+D
                running = 0;
                break;
            }

            if (!running) break;

            // Xoa ky tu xuong dong o cuoi
            buffer[strcspn(buffer, "\n")] = '\0';

            // Cho den khi kenh Server->Client trong de ghi tin nhan
            if (sem_wait(&shared->s2c.sem_empty) == -1) {
                if (errno == EINTR) {
                    if (!running) break;
                    continue;
                }
                break;
            }

            if (!running) break;

            // Sao chep tin nhan vao bo nho chia se
            strncpy(shared->s2c.message, buffer, BUFFER_SIZE - 1);
            shared->s2c.message[BUFFER_SIZE - 1] = '\0';

            // Bao tin nhan moi da san sang cho Client doc
            sem_post(&shared->s2c.sem_filled);

            // Kiem tra neu Server muon chu dong thoat
            if (strcmp(buffer, "/quit") == 0) {
                running = 0;
                break;
            }
        }
    }

    cleanup();
    return 0;
}
