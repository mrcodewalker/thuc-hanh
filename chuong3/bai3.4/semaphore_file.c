/*
 * Bai 3.4: Semaphore File Sync
 * Tao 3 thread, cac thread lan luot tang 1 bien chung them 1 don vi
 * va ghi gia tri moi vao 1 trong 2 file output.
 * Su dung semaphore de dong bo viec ghi vao 2 file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 3
#define NUM_ITERATIONS 6
#define FILE1 "output1.txt"
#define FILE2 "output2.txt"

int counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Semaphore cho 2 file - moi file chi cho 1 thread ghi tai 1 thoi diem */
sem_t sem_file1;
sem_t sem_file2;

FILE *fp1 = NULL;
FILE *fp2 = NULL;

/* Ghi vao file 1 (bao ve boi semaphore) */
void write_file1(int thread_id, int value) {
    sem_wait(&sem_file1);  // P(sem_file1) - chiem semaphore

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "[Thread %d] counter = %d\n", thread_id, value);
    fprintf(fp1, "%s", buffer);
    fflush(fp1);
    printf("  [Thread %d] Ghi vao %s: counter = %d\n", thread_id, FILE1, value);

    sem_post(&sem_file1);  // V(sem_file1) - tra semaphore
}

/* Ghi vao file 2 (bao ve boi semaphore) */
void write_file2(int thread_id, int value) {
    sem_wait(&sem_file2);  // P(sem_file2)

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "[Thread %d] counter = %d\n", thread_id, value);
    fprintf(fp2, "%s", buffer);
    fflush(fp2);
    printf("  [Thread %d] Ghi vao %s: counter = %d\n", thread_id, FILE2, value);

    sem_post(&sem_file2);  // V(sem_file2)
}

void *thread_func(void *arg) {
    int id = *(int *)arg;
    int i;

    for (i = 0; i < NUM_ITERATIONS; i++) {
        // Tang bien chung (mutex bao ve)
        pthread_mutex_lock(&counter_mutex);
        counter++;
        int val = counter;
        pthread_mutex_unlock(&counter_mutex);

        // Ghi vao file: so chan → file1, so le → file2
        if (val % 2 == 0) {
            write_file1(id, val);
        } else {
            write_file2(id, val);
        }

        usleep(50000 + (rand() % 100000)); // 50-150ms random delay
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    int i;

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 3.4 - SEMAPHORE FILE SYNC             ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║ 3 threads tang bien chung + ghi vao 2 file   ║\n");
    printf("║ Su dung semaphore dong bo ghi file           ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    srand(42);

    // Khoi tao semaphore (gia tri 1 = binary semaphore)
    sem_init(&sem_file1, 0, 1);
    sem_init(&sem_file2, 0, 1);

    // Mo 2 file
    fp1 = fopen(FILE1, "w");
    fp2 = fopen(FILE2, "w");
    if (!fp1 || !fp2) {
        perror("[ERROR] Khong the mo file");
        return 1;
    }

    fprintf(fp1, "=== FILE 1 - Gia tri chan ===\n");
    fprintf(fp2, "=== FILE 2 - Gia tri le ===\n");

    printf("[Main] Gia tri ban dau: counter = %d\n", counter);
    printf("[Main] %d threads, moi thread tang %d lan\n", NUM_THREADS, NUM_ITERATIONS);
    printf("[Main] So chan → %s, So le → %s\n\n", FILE1, FILE2);
    printf("── Qua trinh thuc hien ─────────────────────────\n\n");

    // Tao 3 thread
    for (i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }

    // Cho tat ca thread ket thuc
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(fp1);
    fclose(fp2);

    printf("\n── Ket qua ─────────────────────────────────────\n\n");
    printf("[Main] Gia tri cuoi: counter = %d (mong doi: %d)\n",
           counter, NUM_THREADS * NUM_ITERATIONS);

    if (counter == NUM_THREADS * NUM_ITERATIONS)
        printf("[Main] ✓ THANH CONG!\n");
    else
        printf("[Main] ✗ LOI!\n");

    // In noi dung 2 file
    printf("\n── Noi dung %s ──────────────────────────\n\n", FILE1);
    fp1 = fopen(FILE1, "r");
    if (fp1) {
        char line[256];
        while (fgets(line, sizeof(line), fp1))
            printf("    %s", line);
        fclose(fp1);
    }

    printf("\n\n── Noi dung %s ──────────────────────────\n\n", FILE2);
    fp2 = fopen(FILE2, "r");
    if (fp2) {
        char line[256];
        while (fgets(line, sizeof(line), fp2))
            printf("    %s", line);
        fclose(fp2);
    }

    printf("\n");

    // Huy semaphore
    sem_destroy(&sem_file1);
    sem_destroy(&sem_file2);
    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
