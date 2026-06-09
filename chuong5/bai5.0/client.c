/*
 * Bai 5.0: Chat Client-Server tren cung 1 may bang mmap (Luu lich su vao file)
 * CLIENT - Ket noi vao file mmap, nạp lich su va dong bo tin nhan
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

struct chat_db *shared = NULL;
int file_fd = -1;
volatile int running = 1;
pthread_t recv_thread;
int local_read_count = 0;

/* Ham don dep va luu tru phia Client */
void cleanup() {
    printf("\n[Client] Dang don dep va dong bo du lieu...\n");
    running = 0;

    // Huy thread nhan tin nhan
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    if (shared != NULL) {
        shared->client_alive = 0;

        // Bao thuc Server neu ho dang doi tin nhan
        sem_post(&shared->sem_client_new);

        // Dong bo du lieu xuong dia
        msync(shared, sizeof(struct chat_db), MS_SYNC);

        // Huy anh xa mmap
        munmap(shared, sizeof(struct chat_db));
    }

    if (file_fd != -1) {
        close(file_fd);
    }
    printf("[Client] Da ngat ket noi. Tam biet!\n");
}

/* Bo xu ly tin hieu */
void signal_handler(int sig) {
    running = 0;
    if (shared != NULL) {
        shared->client_alive = 0;
        sem_post(&shared->mutex);
        sem_post(&shared->sem_server_new);
        sem_post(&shared->sem_client_new);
    }
}

/* Thread nhan tin nhan tu Server */
void *receive_handler(void *arg) {
    while (running) {
        // Cho tin hieu tin nhan moi tu Server
        if (sem_wait(&shared->sem_server_new) == -1) {
            if (errno == EINTR) {
                if (!running) break;
                continue;
            }
            break;
        }

        if (!running) break;

        // Doc cac tin nhan moi tu vi tri doc hien tai (local_read_count)
        sem_wait(&shared->mutex);
        int current_count = shared->msg_count;
        sem_post(&shared->mutex);

        while (local_read_count < current_count) {
            struct chat_message msg = shared->messages[local_read_count];
            // Chi in ra neu nguoi gui la Server
            if (strcmp(msg.sender, "Server") == 0) {
                printf("\r[Server]: %s\n[Client]: ", msg.text);
                fflush(stdout);
            }
            local_read_count++;
        }

        // Kiem tra neu Server da dong ket noi
        if (!shared->server_alive) {
            printf("\n[!] Server da ngat ket noi.\n");
            running = 0;
            break;
        }
    }
    return NULL;
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.0 - CHAT CLIENT (mmap File-backed)   ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Dang ky tin hieu de ngat an toan
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 1. Mo file lich su chat (file da phai duoc tao boi Server)
    file_fd = open(SHM_FILE, O_RDWR, 0666);
    if (file_fd == -1) {
        perror("[ERROR] connect (server da chay va khoi tao file chua?)");
        return 1;
    }

    // 2. Anh xa (mmap) file chat_history.dat vao bo nho
    shared = (struct chat_db *)mmap(NULL, sizeof(struct chat_db),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(file_fd);
        return 1;
    }

    // 3. Kiem tra xem Server co dang chay hay khong
    if (!shared->server_alive) {
        printf("[ERROR] Server chua san sang hoac da tat.\n");
        munmap(shared, sizeof(struct chat_db));
        close(file_fd);
        return 1;
    }

    // 4. Tai va hien thi toan bo lich su chat tu file
    printf("[Client] ✓ Da tai lich su chat tu file: %s (%d tin nhan)\n", SHM_FILE, shared->msg_count);
    printf("────────────────── LICH SU CHAT ──────────────────\n");
    for (int i = 0; i < shared->msg_count; i++) {
        printf("[%s]: %s\n", shared->messages[i].sender, shared->messages[i].text);
    }
    printf("──────────────────────────────────────────────────\n\n");

    // Thiet lap index doc cuc bo bat dau tu vi tri cu
    local_read_count = shared->msg_count;

    // Bat co bao hieu Client da tham gia
    shared->client_alive = 1;
    printf("[Client] ✓ Da ket noi den server!\n");
    printf("[Client] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 5. Tao thread de nhan tin nhan tu Server
    if (pthread_create(&recv_thread, NULL, receive_handler, NULL) != 0) {
        perror("[ERROR] pthread_create");
        cleanup();
        return 1;
    }

    char buffer[MAX_MSG_LEN];

    // 6. Vong lap gui tin nhan
    while (running) {
        printf("[Client]: ");
        fflush(stdout);

        // select phi chan STDIN de theo doi cờ server_alive lien tuc
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

        // Kiem tra neu Server da dot ngot tat
        if (!shared->server_alive) {
            printf("\n[!] Server da ngat ket noi.\n");
            running = 0;
            break;
        }

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(buffer, MAX_MSG_LEN, stdin) == NULL) {
                // Ctrl+D
                running = 0;
                break;
            }

            if (!running) break;

            buffer[strcspn(buffer, "\n")] = '\0';

            // Ghi tin nhan cua Client vao DB
            sem_wait(&shared->mutex);
            if (shared->msg_count < MAX_MESSAGES) {
                struct chat_message *msg = &shared->messages[shared->msg_count];
                strcpy(msg->sender, "Client");
                strncpy(msg->text, buffer, MAX_MSG_LEN - 1);
                msg->text[MAX_MSG_LEN - 1] = '\0';

                shared->msg_count++;
                local_read_count++; // Client tu dong tang de khong tu in tin nhan cua chinh minh

                sem_post(&shared->mutex);

                // Bao hieu cho Server co tin nhan moi tu Client
                sem_post(&shared->sem_client_new);
            } else {
                printf("[!] Khong the gui. Lich su chat da day (%d tin nhắn)!\n", MAX_MESSAGES);
                sem_post(&shared->mutex);
            }

            if (strcmp(buffer, "/quit") == 0) {
                running = 0;
                break;
            }
        }
    }

    cleanup();
    return 0;
}
