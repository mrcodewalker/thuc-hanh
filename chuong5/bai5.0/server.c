/*
 * Bai 5.0: Chat Client-Server tren cung 1 may bang mmap (Luu lich su vao file)
 * SERVER - Khoi tao file, mmap file va dong bo tin nhan
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

/* Ham don dep va luu tru du lieu ve file truoc khi thoat */
void cleanup() {
    printf("\n[Server] Dang don dep va dong bo du lieu ve dia...\n");
    running = 0;

    // Huy thread nhan tin nhan
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    if (shared != NULL) {
        shared->server_alive = 0;
        
        // Bao thuc Client neu ho dang doi tin nhan de ho tu thoat
        sem_post(&shared->sem_server_new);

        // Huy cac semaphore
        sem_destroy(&shared->mutex);
        sem_destroy(&shared->sem_server_new);
        sem_destroy(&shared->sem_client_new);

        // Dong bo du lieu xuong file dat tin cay
        msync(shared, sizeof(struct chat_db), MS_SYNC);
        
        // Huy anh xa mmap
        munmap(shared, sizeof(struct chat_db));
    }

    if (file_fd != -1) {
        close(file_fd);
    }
    printf("[Server] Hoan tat dong bo lich su chat. Da thoat!\n");
}

/* Bo xu ly tin hieu (Ctrl+C, SIGTERM) */
void signal_handler(int sig) {
    running = 0;
    if (shared != NULL) {
        shared->server_alive = 0;
        // Danh thuc cac thread dang cho semaphore
        sem_post(&shared->mutex);
        sem_post(&shared->sem_server_new);
        sem_post(&shared->sem_client_new);
    }
}

/* Thread nhan tin nhan tu Client */
void *receive_handler(void *arg) {
    while (running) {
        // Cho tin hieu tin nhan moi tu Client
        if (sem_wait(&shared->sem_client_new) == -1) {
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
            // Chi in ra neu la nguoi gui la Client
            if (strcmp(msg.sender, "Client") == 0) {
                printf("\r[Client]: %s\n[Server]: ", msg.text);
                fflush(stdout);
            }
            local_read_count++;
        }

        // Kiem tra neu client da ngat ket noi
        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }
    }
    return NULL;
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.0 - CHAT SERVER (mmap File-backed)   ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Dang ky tin hieu de ngat an toan va tu dong ghi xuong file
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 1. Mo hoac tao file luu tru lich su
    file_fd = open(SHM_FILE, O_RDWR | O_CREAT, 0666);
    if (file_fd == -1) {
        perror("[ERROR] open chat_history.dat");
        return 1;
    }

    // 2. Kiem tra kich thuoc file de xem day co phai file moi khong
    struct stat st;
    if (fstat(file_fd, &st) == -1) {
        perror("[ERROR] fstat");
        close(file_fd);
        return 1;
    }

    int is_new_file = (st.st_size == 0);

    // 3. Khai bao kich thuoc file dung bang struct chat_db
    if (ftruncate(file_fd, sizeof(struct chat_db)) == -1) {
        perror("[ERROR] ftruncate");
        close(file_fd);
        return 1;
    }

    // 4. Anh xa (mmap) file chat_history.dat vao bo nho ao
    shared = (struct chat_db *)mmap(NULL, sizeof(struct chat_db),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(file_fd);
        return 1;
    }

    // 5. Khoi tao hoac doc thong tin cu
    if (is_new_file) {
        shared->msg_count = 0;
        shared->server_alive = 1;
        shared->client_alive = 0;

        // Khoi tao cac semaphore
        sem_init(&shared->mutex, 1, 1);
        sem_init(&shared->sem_server_new, 1, 0);
        sem_init(&shared->sem_client_new, 1, 0);
        
        printf("[Server] Khoi tao file nhat ky chat moi: %s\n", SHM_FILE);
    } else {
        shared->server_alive = 1;
        shared->client_alive = 0;

        // Khoi tao lai cac semaphore (cac gia tri semaphore cu luu tren file co the khong hop le)
        sem_init(&shared->mutex, 1, 1);
        sem_init(&shared->sem_server_new, 1, 0);
        sem_init(&shared->sem_client_new, 1, 0);

        printf("[Server] ✓ Da tai lich su chat tu file: %s (%d tin nhan)\n", SHM_FILE, shared->msg_count);
        printf("────────────────── LICH SU CHAT ──────────────────\n");
        for (int i = 0; i < shared->msg_count; i++) {
            printf("[%s]: %s\n", shared->messages[i].sender, shared->messages[i].text);
        }
        printf("──────────────────────────────────────────────────\n\n");
    }

    // Thiet lap index doc cuc bo bat dau tu vi tri cu
    local_read_count = shared->msg_count;

    printf("[Server] Dang cho client ket noi...\n\n");

    // 6. Cho Client ket noi
    while (running && !shared->client_alive) {
        usleep(100000); // Cho 100ms moi lan kiem tra
    }

    if (!running) {
        cleanup();
        return 0;
    }

    printf("[Server] ✓ Client da ket noi!\n");
    printf("[Server] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 7. Tao thread de nhan tin nhan tu Client
    if (pthread_create(&recv_thread, NULL, receive_handler, NULL) != 0) {
        perror("[ERROR] pthread_create");
        cleanup();
        return 1;
    }

    char buffer[MAX_MSG_LEN];

    // 8. Vong lap nhap/gui tin nhan
    while (running) {
        printf("[Server]: ");
        fflush(stdout);

        // select phi chan STDIN de kiem tra cờ client_alive lien tuc
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

        // Kiem tra xem client co dot ngot tat khong
        if (!shared->client_alive) {
            printf("\n[!] Client da ngat ket noi.\n");
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

            // Ghi tin nhan moi vao DB
            sem_wait(&shared->mutex);
            if (shared->msg_count < MAX_MESSAGES) {
                struct chat_message *msg = &shared->messages[shared->msg_count];
                strcpy(msg->sender, "Server");
                strncpy(msg->text, buffer, MAX_MSG_LEN - 1);
                msg->text[MAX_MSG_LEN - 1] = '\0';
                
                shared->msg_count++;
                local_read_count++; // Server tu dong tang de khong tu in lai tin nhan cua chinh minh
                
                sem_post(&shared->mutex);

                // Bao hieu cho Client co tin nhan moi
                sem_post(&shared->sem_server_new);
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
