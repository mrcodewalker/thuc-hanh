/*
 * Bai 3.1: Mutex Counter
 * Tao 2 thread, ca 2 thread lan luot tang bien dem len 1 don vi
 * Su dung mutex de dong bo viec ghi vao bien do
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_COUNT 10

int counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(void *arg) {
    int id = *(int *)arg;
    int i;

    for (i = 0; i < MAX_COUNT; i++) {
        pthread_mutex_lock(&mutex);

        counter++;
        printf("  [Thread %d] counter = %d\n", id, counter);

        pthread_mutex_unlock(&mutex);
        usleep(100000); // 100ms delay de thay ro thu tu
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    printf("╔══════════════════════════════════════════╗\n");
    printf("║   BAI 3.1 - MUTEX COUNTER DEMO          ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ 2 threads cung tang 1 bien dem           ║\n");
    printf("║ Su dung mutex de dong bo                 ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");

    printf("[Main] Gia tri ban dau: counter = %d\n", counter);
    printf("[Main] Moi thread se tang counter %d lan\n\n", MAX_COUNT);

    pthread_create(&t1, NULL, thread_func, &id1);
    pthread_create(&t2, NULL, thread_func, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\n[Main] Ket qua cuoi cung: counter = %d\n", counter);
    printf("[Main] Gia tri mong doi: %d\n", MAX_COUNT * 2);

    if (counter == MAX_COUNT * 2)
        printf("[Main] ✓ THANH CONG - Mutex hoat dong dung!\n");
    else
        printf("[Main] ✗ LOI - Gia tri khong dung!\n");

    pthread_mutex_destroy(&mutex);
    return 0;
}
