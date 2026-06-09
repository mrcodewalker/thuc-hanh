/*
 * Bai 5.2: Gia lap Client-Server bang Shared Memory va OS Signals (SIGUSR1)
 * SERVER - Khoi tao Shared Memory, ngu bang pause(), va ghi log khi duoc danh thuc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "shared_data.h"

struct shared_data *shared = NULL;
int shm_fd = -1;
volatile int running = 1;
volatile sig_atomic_t sigusr1_received = 0;

/* Ham ghi nhat ky vao file log */
void write_log(const char *name, int tuoi) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("[ERROR] Khong the mo file log");
        return;
    }

    // Lay thoi gian hien tai
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Ghi vao file log va in ra stdout de kiem tra
    fprintf(log_file, "[%s] [INFO] Nhan du lieu: Ten = %s, Tuoi = %d\n", timestamp, name, tuoi);
    fclose(log_file);

    printf("\n[%s] [LOGGED] Da ghi nhan: Ten = %s, Tuoi = %d\n", timestamp, name, tuoi);
}

/* Bo xu ly tin hieu SIGUSR1 (danh thuc server) */
void handle_sigusr1(int sig) {
    sigusr1_received = 1;
}

/* Bo xu ly tin hieu Ctrl+C (SIGINT) va SIGTERM */
void handle_shutdown(int sig) {
    running = 0;
}

/* Don dep tai nguyen Shared Memory */
void cleanup() {
    printf("\n[Server] Dang don dep tai nguyen...\n");
    if (shared != NULL) {
        shared->server_alive = 0;
        munmap(shared, sizeof(struct shared_data));
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    shm_unlink(SHM_NAME);
    printf("[Server] Da giai phong Shared Memory. Tam biet!\n");
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.2 - SERVER (Shared Memory & Signal)  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Dang ky bo xu ly tin hieu
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);

    // Xoa phan vung shm cu neu co loi
    shm_unlink(SHM_NAME);

    // 2. Tao bo nho chia se
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[ERROR] shm_open");
        return 1;
    }

    // 3. Cau hinh kich thuoc
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
        perror("[ERROR] ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // 4. Map bo nho chia se
    shared = (struct shared_data *)mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // Khai bao cac thong tin ban dau
    shared->server_pid = getpid();
    shared->data_ready = 0;
    shared->processed = 0;
    shared->server_alive = 1;

    printf("[Server] PID cua server la: %d\n", shared->server_pid);
    printf("[Server] Shared Memory duoc tao tai: %s\n", SHM_NAME);
    printf("[Server] File log ghi nhan: %s\n", LOG_FILE);
    printf("[Server] Bat dau vao trang thai ngu...\n\n");

    // 5. Vong lap chinh cua Server
    while (running) {
        printf("[Server] Zzz... Dang ngu (pause)... ");
        fflush(stdout);

        // pause() se dung chuong trinh lai hoan toan va khong ton CPU
        // cho den khi co tin hieu gui den
        pause();

        // Kiem tra neu bi ngat boi Ctrl+C
        if (!running) break;

        // Kiem tra neu nhan duoc tin hieu SIGUSR1 tu client
        if (sigusr1_received) {
            printf("\n[Server] (o_o)! Tinh giac do nhan tin hieu SIGUSR1.\n");
            sigusr1_received = 0;

            if (shared->data_ready) {
                // Doc du lieu va ghi vao file log
                write_log(shared->user_data.name, shared->user_data.tuoi);
                
                // Reset co san sang va dat co processed bao cho Client
                shared->data_ready = 0;
                shared->processed = 1;
            } else {
                printf("[Server] Warning: Tinh day nhung khong co du lieu san sang.\n");
            }
        }
    }

    cleanup();
    return 0;
}
