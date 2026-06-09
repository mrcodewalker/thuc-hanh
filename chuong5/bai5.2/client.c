/*
 * Bai 5.2: Gia lap Client-Server bang Shared Memory va OS Signals (SIGUSR1)
 * CLIENT - Nhap ten, tuoi, ghi vao Shared Memory, gui tin hieu danh thuc Server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include "shared_data.h"

struct shared_data *shared = NULL;
int shm_fd = -1;
volatile int running = 1;

void handle_shutdown(int sig) {
    running = 0;
}

void cleanup() {
    printf("\n[Client] Dang don dep tai nguyen...\n");
    if (shared != NULL) {
        munmap(shared, sizeof(struct shared_data));
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    printf("[Client] Da dong. Tam biet!\n");
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 5.2 - CLIENT (Shared Memory & Signal)  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Dang ky tin hieu don dep khi nguoi dung an Ctrl+C
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);

    // 1. Mo phan vung bo nho chia se tu Server
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[ERROR] connect (server da chay chua?)");
        return 1;
    }

    // 2. Map bo nho chia se vao khong gian dia chi
    shared = (struct shared_data *)mmap(NULL, sizeof(struct shared_data),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("[ERROR] mmap");
        close(shm_fd);
        return 1;
    }

    // 3. Kiem tra xem Server co dang hoat dong khong
    if (!shared->server_alive) {
        printf("[ERROR] Server chua chay hoac da dung.\n");
        munmap(shared, sizeof(struct shared_data));
        close(shm_fd);
        return 1;
    }

    printf("[Client] ✓ Ket noi den Shared Memory cua Server thanh cong.\n");
    printf("[Client] Server PID: %d\n", shared->server_pid);
    printf("[Client] Nhap thong tin duoi day. Go '/quit' de thoat.\n\n");

    char name_buf[100];
    char age_buf[32];

    // 4. Vong lap nhap du lieu va gui tin hieu
    while (running) {
        // Kiem tra server
        if (!shared->server_alive) {
            printf("\n[!] Server da ngat ket noi.\n");
            break;
        }

        // Nhap Ten
        printf("[Client] Nhap ten: ");
        if (fgets(name_buf, sizeof(name_buf), stdin) == NULL) {
            break;
        }
        
        if (!running) break;

        // Xoa ky tu xuong dong o cuoi
        name_buf[strcspn(name_buf, "\n")] = '\0';

        if (strcmp(name_buf, "/quit") == 0) {
            break;
        }

        if (strlen(name_buf) == 0) {
            printf("[!] Ten khong duoc de trong. Vui long nhap lai.\n");
            continue;
        }

        if (strlen(name_buf) >= 25) {
            printf("[!] Ten qua dai (toi da 24 ky tu). Vui long nhap lai.\n");
            continue;
        }

        // Nhap Tuoi
        int tuoi = 0;
        int age_valid = 0;
        while (running && !age_valid) {
            if (!shared->server_alive) {
                break;
            }

            printf("[Client] Nhap tuoi: ");
            if (fgets(age_buf, sizeof(age_buf), stdin) == NULL) {
                break;
            }

            if (!running) break;

            age_buf[strcspn(age_buf, "\n")] = '\0';
            if (strcmp(age_buf, "/quit") == 0) {
                running = 0;
                break;
            }

            // Kiem tra xem nhap vao co phai la so nguyen duong khong
            char *endptr;
            tuoi = strtol(age_buf, &endptr, 10);
            if (endptr == age_buf || *endptr != '\0' || tuoi <= 0) {
                printf("[!] Tuoi phai la mot so nguyen duong. Vui long nhap lai.\n");
                continue;
            }
            age_valid = 1;
        }

        if (!running) break;

        // Ghi thong tin vao Shared Memory
        strncpy(shared->user_data.name, name_buf, sizeof(shared->user_data.name) - 1);
        shared->user_data.name[sizeof(shared->user_data.name) - 1] = '\0';
        shared->user_data.tuoi = tuoi;

        shared->data_ready = 1;
        shared->processed = 0;

        printf("[Client] Dang gui tin hieu SIGUSR1 den Server (PID: %d)...\n", shared->server_pid);
        
        // 5. Gui signal SIGUSR1 de wake up server day
        if (kill(shared->server_pid, SIGUSR1) == -1) {
            perror("[ERROR] kill signal");
            break;
        }

        printf("[Client] Dang cho Server ghi log... ");
        fflush(stdout);

        // 6. Cho phan hoi tu Server
        while (running && !shared->processed) {
            if (!shared->server_alive) {
                printf("\n[!] Server da bi ngat giua chung.\n");
                running = 0;
                break;
            }
            usleep(10000); // Cho 10ms moi lan quet
        }

        if (!running) break;

        printf("✓ Server da ghi log thanh cong!\n\n");
    }

    cleanup();
    return 0;
}
