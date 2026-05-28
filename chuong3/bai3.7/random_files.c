/*
 * Bai 3.7: Tao 10 file, moi file chua 5 trieu so ngau nhien (1 chu so)
 * So sanh thoi gian: multithread vs single thread
 *
 * Moi file: 5,000,000 so ngau nhien (0-9), moi so 1 dong
 * Tong: 10 file x 5 trieu = 50 trieu so
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define NUM_FILES 10
#define NUMS_PER_FILE 5000000  // 5 trieu
#define NUM_THREADS 10         // 1 thread / file
#define BUFFER_SIZE 65536      // 64KB write buffer

typedef struct {
    int file_id;
    unsigned int seed;
} ThreadData;

/* Tao 1 file voi 5 trieu so ngau nhien */
void create_random_file(int file_id, unsigned int seed) {
    char filename[64];
    snprintf(filename, sizeof(filename), "data/random_%02d.txt", file_id);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("[ERROR] Khong the tao file: %s\n", filename);
        return;
    }

    // Dung buffer lon de ghi nhanh hon
    char buffer[BUFFER_SIZE];
    int buf_pos = 0;
    unsigned int local_seed = seed;

    for (int i = 0; i < NUMS_PER_FILE; i++) {
        int digit = rand_r(&local_seed) % 10;
        buffer[buf_pos++] = '0' + digit;
        buffer[buf_pos++] = '\n';

        // Flush buffer khi day
        if (buf_pos >= BUFFER_SIZE - 2) {
            fwrite(buffer, 1, buf_pos, fp);
            buf_pos = 0;
        }
    }

    // Ghi phan con lai
    if (buf_pos > 0) {
        fwrite(buffer, 1, buf_pos, fp);
    }

    fclose(fp);
}

/* Thread function */
void *thread_create_file(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    create_random_file(data->file_id, data->seed);
    return NULL;
}

double get_time_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

/* Xoa cac file cu */
void cleanup_files() {
    char filename[64];
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(filename, sizeof(filename), "data/random_%02d.txt", i);
        remove(filename);
    }
}

int main() {
    struct timespec t_start, t_end;
    double time_single, time_multi;

    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   BAI 3.7 - TAO FILE SO NGAU NHIEN (MULTITHREAD)        ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ %d files x %d so/file = %d trieu so          ║\n",
           NUM_FILES, NUMS_PER_FILE, NUM_FILES * NUMS_PER_FILE / 1000000);
    printf("║ So sanh: Single thread vs Multi thread (%d threads)      ║\n", NUM_THREADS);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    // Tao thu muc data
    system("mkdir -p data");

    // ═══════════════════════════════════════════
    // TRUONG HOP 1: KHONG MULTITHREAD
    // ═══════════════════════════════════════════
    printf("── [1] KHONG MULTITHREAD (Single Thread) ───────────────────\n\n");
    printf("  Dang tao %d files...\n", NUM_FILES);

    cleanup_files();

    clock_gettime(CLOCK_MONOTONIC, &t_start);

    for (int i = 0; i < NUM_FILES; i++) {
        create_random_file(i, 42 + i);
        printf("  [File %02d] Xong - %d so da ghi\n", i, NUMS_PER_FILE);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    time_single = get_time_ms(&t_start, &t_end);

    printf("\n  Thoi gian: %.1f ms (%.3f giay)\n\n", time_single, time_single / 1000.0);

    // ═══════════════════════════════════════════
    // TRUONG HOP 2: CO MULTITHREAD
    // ═══════════════════════════════════════════
    printf("── [2] CO MULTITHREAD (%d Threads) ─────────────────────────\n\n", NUM_THREADS);
    printf("  Dang tao %d files song song...\n", NUM_FILES);

    cleanup_files();

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    clock_gettime(CLOCK_MONOTONIC, &t_start);

    for (int i = 0; i < NUM_THREADS; i++) {
        data[i].file_id = i;
        data[i].seed = 42 + i;
        pthread_create(&threads[i], NULL, thread_create_file, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("  [File %02d] Xong - %d so da ghi\n", i, NUMS_PER_FILE);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    time_multi = get_time_ms(&t_start, &t_end);

    printf("\n  Thoi gian: %.1f ms (%.3f giay)\n\n", time_multi, time_multi / 1000.0);

    // ═══════════════════════════════════════════
    // SO SANH
    // ═══════════════════════════════════════════
    printf("── SO SANH KET QUA ─────────────────────────────────────────\n\n");
    printf("  ┌─────────────────────┬──────────────────┬───────────────┐\n");
    printf("  │ Phuong phap         │ Thoi gian (ms)   │ Thoi gian (s) │\n");
    printf("  ├─────────────────────┼──────────────────┼───────────────┤\n");
    printf("  │ Single thread       │ %12.1f ms │ %9.3f s │\n", time_single, time_single / 1000.0);
    printf("  │ Multi thread (%2dt)  │ %12.1f ms │ %9.3f s │\n", NUM_THREADS, time_multi, time_multi / 1000.0);
    printf("  └─────────────────────┴──────────────────┴───────────────┘\n\n");

    double speedup = time_single / time_multi;
    printf("  ⚡ Speedup: %.2fx ", speedup);
    if (speedup > 1.0)
        printf("(Multithread NHANH hon)\n");
    else
        printf("(Single thread nhanh hon - I/O bottleneck)\n");

    printf("\n  Thong tin file:\n");
    printf("  - So luong file: %d\n", NUM_FILES);
    printf("  - So/file: %d\n", NUMS_PER_FILE);
    printf("  - Tong so: %d\n", NUM_FILES * NUMS_PER_FILE);
    printf("  - Kich thuoc moi file: ~%.1f MB\n", NUMS_PER_FILE * 2.0 / (1024 * 1024));

    // Kiem tra 1 file mau
    printf("\n  Mau 10 so dau cua file random_00.txt:\n    ");
    FILE *fp = fopen("data/random_00.txt", "r");
    if (fp) {
        char line[16];
        for (int i = 0; i < 10 && fgets(line, sizeof(line), fp); i++) {
            line[strcspn(line, "\n")] = '\0';
            printf("%s ", line);
        }
        fclose(fp);
    }
    printf("\n");

    return 0;
}
