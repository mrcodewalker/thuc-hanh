/*
 * Bai 6.5: Chat Server su dung Shared Memory va lien ket Shared Library
 * Nhan duong dan Shm va Log tu file config.
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
#include "shared_chat.h"
#include "chat_lib.h"

struct shared_data *shared = NULL;
int shm_fd = -1;
volatile int running = 1;
pthread_t recv_thread;

char shm_name[MAX_SHM_LEN] = "/shm_chat_pkg_def";
char log_path[MAX_PATH_LEN] = "/tmp/chat_pkg_def.log";

/* Ham don dep khi thoat */
void cleanup() {
    printf("\n[Server] Dang giai phong tai nguyen...\n");
    running = 0;

    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    if (shared != NULL) {
        shared->server_alive = 0;
        sem_destroy(&shared->s2c.sem_filled);
        sem_destroy(&shared->s2c.sem_empty);
        sem_destroy(&shared->c2s.sem_filled);
        sem_destroy(&shared->c2s.sem_empty);
        
        munmap(shared, sizeof(struct shared_data));
    }

    if (shm_fd != -1) {
        close(shm_fd);
    }
    
    shm_unlink(shm_name);
    printf("[Server] Da go bo Shared Memory. Tam biet!\n");
}

/* Bo xu ly tin hieu Ctrl+C */
void signal_handler(int sig) {
    running = 0;
    if (shared != NULL) {
        shared->server_alive = 0;
        sem_post(&shared->s2c.sem_filled);
        sem_post(&shared->s2c.sem_empty);
        sem_post(&shared->c2s.sem_filled);
        sem_post(&shared->c2s.sem_empty);
    }
}

/* Thread nhan tin nhan tu Client */
void *receive_handler(void *arg) {
    while (running) {
        if (sem_wait(&shared->c2s.sem_filled) == -1) {
            if (errno == EINTR) {
                if (!running) break;
                continue;
            }
            break;
        }

        if (!running) break;

        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }

        if (strcmp(shared->c2s.message, "/quit") == 0) {
            printf("\n[!] Client da thoat.\n");
            running = 0;
            break;
        }

        // In tin nhan
        printf("\r[Client]: %s\n[Server]: ", shared->c2s.message);
        fflush(stdout);

        // Ghi log (Goi ham tu Shared Library)
        write_log(log_path, "Client", shared->c2s.message);

        sem_post(&shared->c2s.sem_empty);
    }
    return NULL;
}

int main() {
    char buffer[BUFFER_SIZE];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 6.5 - PACKAGED CHAT SERVER             ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Doc file cấu hình
    printf("[1] Dang nap cau hinh tu file...\n");
    if (read_config("/etc/chat.conf", shm_name, log_path) == 0) {
        printf("    ✓ Nap cau hinh thanh cong tu /etc/chat.conf\n");
    } else if (read_config("config/chat.conf", shm_name, log_path) == 0) {
        printf("    ✓ Nap cau hinh thanh cong tu file cuc bo config/chat.conf\n");
    } else {
        printf("    [!] Khong tim thay file cau hinh. Su dung thong so mac dinh.\n");
    }
    printf("    -> SHM Name: %s\n", shm_name);
    printf("    -> Log Path: %s\n\n", log_path);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 2. Tao Shared Memory
    shm_unlink(shm_name);
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[ERROR] shm_open");
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
        perror("[ERROR] ftruncate");
        close(shm_fd);
        shm_unlink(shm_name);
        return 1;
    }

    shared = (struct shared_data *)mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(shm_fd);
        shm_unlink(shm_name);
        return 1;
    }

    shared->server_alive = 1;
    shared->client_alive = 0;

    // Khoi tao semaphores
    sem_init(&shared->s2c.sem_filled, 1, 0);
    sem_init(&shared->s2c.sem_empty, 1, 1);
    sem_init(&shared->c2s.sem_filled, 1, 0);
    sem_init(&shared->c2s.sem_empty, 1, 1);

    printf("[Server] Dang cho client ket noi...\n\n");
    while (running && !shared->client_alive) {
        usleep(100000);
    }

    if (!running) {
        cleanup();
        return 0;
    }

    printf("[Server] ✓ Client da ket noi!\n");
    printf("[Server] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // Ghi log phien lam viec moi
    write_log(log_path, "SYSTEM", "--- Bắt đầu phiên Chat Server mới ---");

    // 3. Tao thread nhan tin nhan
    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    // 4. Vong lap nhap gui tin
    while (running) {
        printf("[Server]: ");
        fflush(stdout);

        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

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

        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                running = 0;
                break;
            }

            if (!running) break;

            buffer[strcspn(buffer, "\n")] = '\0';

            if (sem_wait(&shared->s2c.sem_empty) == -1) {
                if (errno == EINTR) {
                    if (!running) break;
                    continue;
                }
                break;
            }

            if (!running) break;

            strncpy(shared->s2c.message, buffer, BUFFER_SIZE - 1);
            shared->s2c.message[BUFFER_SIZE - 1] = '\0';

            sem_post(&shared->s2c.sem_filled);

            // Ghi log tin nhan
            write_log(log_path, "Server", buffer);

            if (strcmp(buffer, "/quit") == 0) {
                running = 0;
                break;
            }
        }
    }

    cleanup();
    return 0;
}
