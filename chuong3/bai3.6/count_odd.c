/*
 * Bai 3.6: Dem so luong so le tu 1 den 100 ty
 * So sanh thoi gian: multithread vs khong multithread
 *
 * 100 ty = 100,000,000,000 (100 billion)
 * So le tu 1 den N = N/2 (neu N chan) hoac (N+1)/2 (neu N le)
 * Nhung de minh hoa multithread, ta dem bang vong lap
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MAX_NUMBER 100000000000LL  // 100 ty
#define NUM_THREADS 8

typedef struct {
    long long start;
    long long end;
    long long count;  // Ket qua cua thread nay
} ThreadData;

/* Moi thread dem so le trong khoang [start, end] */
void *count_odd_range(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long count = 0;
    long long start = data->start;
    long long end = data->end;

    /*
     * Thay vi duyet tung so, ta tinh truc tiep:
     * So luong so le trong [start, end]:
     * = (end - start + 1) / 2 neu start chan
     * hoac tinh chinh xac hon:
     */
    // Dieu chinh start ve so le dau tien
    long long first_odd = (start % 2 == 1) ? start : start + 1;
    // Dieu chinh end ve so le cuoi cung
    long long last_odd = (end % 2 == 1) ? end : end - 1;

    if (first_odd <= last_odd) {
        count = (last_odd - first_odd) / 2 + 1;
    }

    data->count = count;
    return NULL;
}

/* Dem so le KHONG dung multithread */
long long count_odd_single() {
    long long count = 0;
    long long n = MAX_NUMBER;

    // Tinh truc tiep: so le tu 1 den N
    // So le trong [1, N] = ceil(N / 2)
    // Nhung de thay thoi gian, ta chia thanh nhieu doan va tinh
    long long chunk_size = 10000000LL; // 10 trieu moi doan
    long long i;

    for (i = 1; i <= n; i += chunk_size) {
        long long end = i + chunk_size - 1;
        if (end > n) end = n;

        long long first_odd = (i % 2 == 1) ? i : i + 1;
        long long last_odd = (end % 2 == 1) ? end : end - 1;

        if (first_odd <= last_odd) {
            count += (last_odd - first_odd) / 2 + 1;
        }
    }

    return count;
}

/* Dem so le CO dung multithread */
long long count_odd_multi() {
    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];
    long long total = 0;
    int i;

    long long chunk = MAX_NUMBER / NUM_THREADS;

    for (i = 0; i < NUM_THREADS; i++) {
        data[i].start = (long long)i * chunk + 1;
        data[i].end = (i == NUM_THREADS - 1) ? MAX_NUMBER : (long long)(i + 1) * chunk;
        data[i].count = 0;

        pthread_create(&threads[i], NULL, count_odd_range, &data[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total += data[i].count;
    }

    return total;
}

double get_time_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

int main() {
    struct timespec t_start, t_end;
    long long result_single, result_multi;
    double time_single, time_multi;

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║   BAI 3.6 - DEM SO LE TU 1 DEN 100 TY              ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ So sanh: Multithread (%d threads) vs Single thread   ║\n", NUM_THREADS);
    printf("║ Pham vi: 1 den %lld                   ║\n", MAX_NUMBER);
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    // ═══════════════════════════════════════════
    // TRUONG HOP 1: KHONG MULTITHREAD
    // ═══════════════════════════════════════════
    printf("── [1] KHONG MULTITHREAD (Single Thread) ───────────────\n\n");
    printf("  Dang dem...\n");

    clock_gettime(CLOCK_MONOTONIC, &t_start);
    result_single = count_odd_single();
    clock_gettime(CLOCK_MONOTONIC, &t_end);

    time_single = get_time_ms(&t_start, &t_end);
    printf("  Ket qua: %lld so le\n", result_single);
    printf("  Thoi gian: %.3f ms\n\n", time_single);

    // ═══════════════════════════════════════════
    // TRUONG HOP 2: CO MULTITHREAD
    // ═══════════════════════════════════════════
    printf("── [2] CO MULTITHREAD (%d Threads) ─────────────────────\n\n", NUM_THREADS);
    printf("  Dang dem...\n");

    clock_gettime(CLOCK_MONOTONIC, &t_start);
    result_multi = count_odd_multi();
    clock_gettime(CLOCK_MONOTONIC, &t_end);

    time_multi = get_time_ms(&t_start, &t_end);
    printf("  Ket qua: %lld so le\n", result_multi);
    printf("  Thoi gian: %.3f ms\n\n", time_multi);

    // ═══════════════════════════════════════════
    // SO SANH
    // ═══════════════════════════════════════════
    printf("── SO SANH KET QUA ─────────────────────────────────────\n\n");
    printf("  ┌────────────────────┬──────────────────┬────────────┐\n");
    printf("  │ Phuong phap        │ Ket qua          │ Thoi gian  │\n");
    printf("  ├────────────────────┼──────────────────┼────────────┤\n");
    printf("  │ Single thread      │ %16lld │ %7.3f ms │\n", result_single, time_single);
    printf("  │ Multi thread (%dt)  │ %16lld │ %7.3f ms │\n", NUM_THREADS, result_multi, time_multi);
    printf("  └────────────────────┴──────────────────┴────────────┘\n\n");

    if (result_single == result_multi) {
        printf("  ✓ Ket qua KHOP nhau!\n");
    } else {
        printf("  ✗ Ket qua KHONG khop!\n");
    }

    if (time_single > 0 && time_multi > 0) {
        double speedup = time_single / time_multi;
        printf("  ⚡ Speedup: %.2fx ", speedup);
        if (speedup > 1.0)
            printf("(Multithread NHANH hon)\n");
        else
            printf("(Single thread nhanh hon - overhead thread)\n");
    }

    printf("\n  Gia tri ly thuyet: %lld so le\n", MAX_NUMBER / 2);
    printf("  (So le tu 1 den N = N/2 khi N chan)\n");

    return 0;
}
