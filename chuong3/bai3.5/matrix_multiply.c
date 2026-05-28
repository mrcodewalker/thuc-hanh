/*
 * Bai 3.5: Matrix Multiplication with Multi-thread
 * Tao 10 ma tran kich thuoc 200x200
 * Nhan 10 ma tran do voi nhau su dung multi thread
 *
 * Chien luoc: Nhan tuan tu A1*A2*...*A10
 * Moi phep nhan ma tran duoc chia thanh nhieu thread
 * Moi thread tinh 1 phan cac hang cua ma tran ket qua
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define SIZE 200
#define NUM_MATRICES 10
#define NUM_THREADS 4

typedef double Matrix[SIZE][SIZE];

/* Du lieu truyen cho moi thread */
typedef struct {
    Matrix *A;        // Ma tran thu nhat
    Matrix *B;        // Ma tran thu hai
    Matrix *C;        // Ma tran ket qua
    int start_row;    // Hang bat dau
    int end_row;      // Hang ket thuc (exclusive)
} ThreadData;

/* Moi thread tinh 1 phan cac hang cua C = A * B */
void *multiply_rows(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int i, j, k;

    for (i = data->start_row; i < data->end_row; i++) {
        for (j = 0; j < SIZE; j++) {
            double sum = 0.0;
            for (k = 0; k < SIZE; k++) {
                sum += (*data->A)[i][k] * (*data->B)[k][j];
            }
            (*data->C)[i][j] = sum;
        }
    }

    return NULL;
}

/* Nhan 2 ma tran su dung nhieu thread */
void matrix_multiply_parallel(Matrix *A, Matrix *B, Matrix *C) {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int i;
    int rows_per_thread = SIZE / NUM_THREADS;

    for (i = 0; i < NUM_THREADS; i++) {
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == NUM_THREADS - 1) ? SIZE : (i + 1) * rows_per_thread;

        pthread_create(&threads[i], NULL, multiply_rows, &thread_data[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

/* Khoi tao ma tran voi gia tri ngau nhien nho */
void init_matrix(Matrix *M) {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            (*M)[i][j] = (double)(rand() % 10) / 10.0; // 0.0 - 0.9
        }
    }
}

/* Copy ma tran */
void copy_matrix(Matrix *dest, Matrix *src) {
    memcpy(dest, src, sizeof(Matrix));
}

/* In 1 goc nho cua ma tran (5x5) */
void print_corner(Matrix *M, const char *name) {
    int i, j;
    printf("  %s (goc 5x5):\n", name);
    for (i = 0; i < 5; i++) {
        printf("    ");
        for (j = 0; j < 5; j++) {
            printf("%8.2f ", (*M)[i][j]);
        }
        printf("...\n");
    }
    printf("    ...\n");
}

int main() {
    int i;
    clock_t start, end;
    double elapsed;

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   BAI 3.5 - MATRIX MULTIPLICATION (MULTITHREAD) ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ 10 ma tran %dx%d, nhan tuan tu A1*A2*...*A10   ║\n", SIZE, SIZE);
    printf("║ Moi phep nhan dung %d threads song song          ║\n", NUM_THREADS);
    printf("╚══════════════════════════════════════════════════╝\n\n");

    srand(42);

    // Cap phat 10 ma tran + 2 buffer
    printf("[Main] Cap phat bo nho cho %d ma tran %dx%d...\n", NUM_MATRICES, SIZE, SIZE);

    Matrix *matrices = (Matrix *)malloc(NUM_MATRICES * sizeof(Matrix));
    Matrix *temp = (Matrix *)malloc(sizeof(Matrix));
    Matrix *result = (Matrix *)malloc(sizeof(Matrix));

    if (!matrices || !temp || !result) {
        printf("[ERROR] Khong du bo nho!\n");
        return 1;
    }

    // Khoi tao 10 ma tran
    printf("[Main] Khoi tao %d ma tran voi gia tri ngau nhien...\n\n", NUM_MATRICES);
    for (i = 0; i < NUM_MATRICES; i++) {
        init_matrix(&matrices[i]);
    }

    // In goc ma tran dau tien
    print_corner(&matrices[0], "Ma tran A1");
    printf("\n");

    // Nhan tuan tu: result = A1 * A2 * A3 * ... * A10
    printf("── Bat dau nhan ma tran ────────────────────────────\n\n");

    start = clock();

    // Buoc 1: result = A1 * A2
    printf("  [Step 1/9] A1 * A2 ...");
    fflush(stdout);
    matrix_multiply_parallel(&matrices[0], &matrices[1], result);
    printf(" done\n");

    // Buoc 2-9: result = result * A(i+1)
    for (i = 2; i < NUM_MATRICES; i++) {
        printf("  [Step %d/9] result * A%d ...", i, i + 1);
        fflush(stdout);

        copy_matrix(temp, result);
        matrix_multiply_parallel(temp, &matrices[i], result);

        printf(" done\n");
    }

    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\n── Ket qua ─────────────────────────────────────────\n\n");
    printf("[Main] ✓ HOAN THANH!\n");
    printf("[Main] Thoi gian: %.3f giay\n", elapsed);
    printf("[Main] So threads moi phep nhan: %d\n", NUM_THREADS);
    printf("[Main] Tong so phep nhan: 9 (A1*A2*...*A10)\n\n");

    // In goc ket qua
    print_corner(result, "Ma tran ket qua");

    printf("\n[Main] Kich thuoc moi ma tran: %dx%d = %d phan tu\n",
           SIZE, SIZE, SIZE * SIZE);
    printf("[Main] Tong bo nho su dung: ~%.1f MB\n",
           (NUM_MATRICES + 2) * sizeof(Matrix) / (1024.0 * 1024.0));

    // Giai phong
    free(matrices);
    free(temp);
    free(result);

    return 0;
}
