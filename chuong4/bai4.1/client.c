/*
 * Bai 4.1: Chat Client-Server tren cung 1 may (localhost)
 * CLIENT - ket noi den server tai 127.0.0.1
 *
 * Su dung 2 thread:
 *  - 1 thread nhan tin nhan tu server
 *  - thread chinh gui tin nhan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int sock = -1;
volatile int running = 1;

/* Thread nhan tin nhan tu server */
void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    int n;

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("\n[!] Server da ngat ket noi.\n");
            running = 0;
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "/quit") == 0) {
            printf("\n[!] Server da thoat.\n");
            running = 0;
            break;
        }

        printf("\r[Server]: %s\n[Client]: ", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.1 - CHAT CLIENT (Localhost)         ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Tao socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[ERROR] socket");
        return 1;
    }

    // 2. Cau hinh dia chi server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 3. Ket noi den server
    printf("[Client] Dang ket noi den 127.0.0.1:%d ...\n", PORT);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] connect (server da chay chua?)");
        close(sock);
        return 1;
    }

    printf("[Client] ✓ Da ket noi den server!\n");
    printf("[Client] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 4. Tao thread nhan tin nhan
    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    // 5. Vong lap gui tin nhan
    while (running) {
        printf("[Client]: ");
        fflush(stdout);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        if (!running) break;

        send(sock, buffer, strlen(buffer), 0);

        char tmp[BUFFER_SIZE];
        strcpy(tmp, buffer);
        tmp[strcspn(tmp, "\n")] = '\0';
        if (strcmp(tmp, "/quit") == 0) {
            running = 0;
            break;
        }
    }

    running = 0;
    pthread_cancel(recv_thread);
    close(sock);
    printf("\n[Client] Da dong ket noi.\n");
    return 0;
}
