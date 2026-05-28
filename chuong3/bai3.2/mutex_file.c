/*
 * Bai 3.2: Mutex File Write
 * Tao 2 thread, ca 2 ghi vao chung 1 file
 * Su dung mutex de bao ve file
 * Su dung wrapper function thay cho ham ghi file thong thuong
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_FILE "output.txt"
#define NUM_WRITES 5

FILE *fp = NULL;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Wrapper function de ghi file an toan voi mutex */
void safe_write(const char *msg) {
    pthread_mutex_lock(&file_mutex);

    if (fp != NULL) {
        fprintf(fp, "%s", msg);
        fflush(fp);
        printf("  [Write] %s", msg);
    }

    pthread_mutex_unlock(&file_mutex);
}

void *writer_thread(void *arg) {
    int id = *(int *)arg;
    int i;
    char buffer[256];

    for (i = 1; i <= NUM_WRITES; i++) {
        snprintf(buffer, sizeof(buffer),
                 "[Thread %d] Dong thu %d - Xin chao tu thread %d!\n",
                 id, i, id);
        safe_write(buffer);
        usleep(150000); // 150ms delay
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    printf("╔══════════════════════════════════════════╗\n");
    printf("║   BAI 3.2 - MUTEX FILE WRITE            ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ 2 threads cung ghi vao 1 file            ║\n");
    printf("║ Su dung mutex + wrapper function         ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    // Mo file
    fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("[ERROR] Khong the mo file");
        return 1;
    }

    printf("[Main] Da mo file: %s\n", OUTPUT_FILE);
    printf("[Main] Moi thread se ghi %d dong\n\n", NUM_WRITES);
    printf("── Noi dung dang ghi vao file ──────────────\n\n");

    pthread_create(&t1, NULL, writer_thread, &id1);
    pthread_create(&t2, NULL, writer_thread, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    fclose(fp);
    fp = NULL;

    printf("\n── Ket qua ─────────────────────────────────\n\n");
    printf("[Main] Da ghi xong vao file: %s\n", OUTPUT_FILE);
    printf("[Main] Noi dung file:\n\n");

    // Doc lai file va in ra
    fp = fopen(OUTPUT_FILE, "r");
    if (fp != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
            printf("    %s", line);
        }
        fclose(fp);
    }

    printf("\n\n[Main] ✓ THANH CONG - File duoc ghi an toan!\n");

    pthread_mutex_destroy(&file_mutex);
    return 0;
}
