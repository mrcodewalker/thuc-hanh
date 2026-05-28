/*
 * Bai 3.3: Mutex Combined (Counter + File)
 * Ket hop ca 2 bai truoc:
 * - 2 thread tang bien dem (mutex dong bo)
 * - 2 thread ghi ket qua vao file (mutex bao ve file)
 * Su dung wrapper function
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_FILE "result.txt"
#define MAX_COUNT 8

int counter = 0;
FILE *fp = NULL;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Wrapper function: tang counter an toan */
int safe_increment(int thread_id) {
    int val;
    pthread_mutex_lock(&counter_mutex);
    counter++;
    val = counter;
    pthread_mutex_unlock(&counter_mutex);
    return val;
}

/* Wrapper function: ghi file an toan */
void safe_log(const char *msg) {
    pthread_mutex_lock(&file_mutex);
    if (fp != NULL) {
        fprintf(fp, "%s", msg);
        fflush(fp);
    }
    pthread_mutex_unlock(&file_mutex);
}

void *worker_thread(void *arg) {
    int id = *(int *)arg;
    int i;
    char buffer[256];

    for (i = 0; i < MAX_COUNT; i++) {
        // Tang counter
        int val = safe_increment(id);

        // In ra man hinh
        printf("  [Thread %d] counter = %d\n", id, val);

        // Ghi vao file
        snprintf(buffer, sizeof(buffer),
                 "[Thread %d] Da tang counter len %d\n", id, val);
        safe_log(buffer);

        usleep(80000); // 80ms delay
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    printf("╔══════════════════════════════════════════╗\n");
    printf("║   BAI 3.3 - MUTEX COMBINED DEMO         ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ 2 threads: tang counter + ghi file       ║\n");
    printf("║ Su dung 2 mutex + wrapper functions      ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    // Mo file log
    fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("[ERROR] Khong the mo file");
        return 1;
    }

    fprintf(fp, "=== BAI 3.3 - KET QUA CHAY ===\n\n");

    printf("[Main] Gia tri ban dau: counter = %d\n", counter);
    printf("[Main] Moi thread tang %d lan, ghi ket qua vao %s\n\n", MAX_COUNT, OUTPUT_FILE);
    printf("── Qua trinh thuc hien ─────────────────────\n\n");

    pthread_create(&t1, NULL, worker_thread, &id1);
    pthread_create(&t2, NULL, worker_thread, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Ghi ket qua cuoi vao file
    char final_msg[256];
    snprintf(final_msg, sizeof(final_msg),
             "\n=== KET QUA CUOI: counter = %d (mong doi: %d) ===\n",
             counter, MAX_COUNT * 2);
    safe_log(final_msg);

    fclose(fp);
    fp = NULL;

    printf("\n── Ket qua ─────────────────────────────────\n\n");
    printf("[Main] Gia tri cuoi: counter = %d\n", counter);
    printf("[Main] Gia tri mong doi: %d\n", MAX_COUNT * 2);

    if (counter == MAX_COUNT * 2)
        printf("[Main] ✓ THANH CONG - Ca 2 mutex hoat dong dung!\n");
    else
        printf("[Main] ✗ LOI - Co van de voi dong bo!\n");

    printf("\n[Main] Noi dung file %s:\n\n", OUTPUT_FILE);

    // Doc lai file
    fp = fopen(OUTPUT_FILE, "r");
    if (fp != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
            printf("    %s", line);
        }
        fclose(fp);
    }

    printf("\n");
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&file_mutex);
    return 0;
}
