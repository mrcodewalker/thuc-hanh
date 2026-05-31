/*
 * Bai 4.1: Chat Client-Server tren cung 1 may (localhost)
 * SERVER - su dung TCP socket, lang nghe tren 127.0.0.1
 *
 * Su dung 2 thread:
 *  - 1 thread nhan tin nhan tu client
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

int client_sock = -1;
volatile int running = 1;

/* Thread nhan tin nhan tu client */
void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    int n;

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("\n[!] Client da ngat ket noi.\n");
            running = 0;
            break;
        }
        // Xoa newline cuoi
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "/quit") == 0) {
            printf("\n[!] Client da thoat.\n");
            running = 0;
            break;
        }

        printf("\r[Client]: %s\n[Server]: ", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.1 - CHAT SERVER (Localhost)         ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Tao socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[ERROR] socket");
        return 1;
    }

    // Cho phep tai su dung dia chi
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Cau hinh dia chi server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 3. Bind socket voi dia chi
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] bind");
        close(server_fd);
        return 1;
    }

    // 4. Lang nghe ket noi
    if (listen(server_fd, 1) < 0) {
        perror("[ERROR] listen");
        close(server_fd);
        return 1;
    }

    printf("[Server] Dang lang nghe tai 127.0.0.1:%d ...\n", PORT);
    printf("[Server] Dang cho client ket noi...\n\n");

    // 5. Chap nhan ket noi
    client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("[ERROR] accept");
        close(server_fd);
        return 1;
    }

    printf("[Server] ✓ Client da ket noi!\n");
    printf("[Server] Go tin nhan va Enter de gui. Go '/quit' de thoat.\n\n");

    // 6. Tao thread nhan tin nhan
    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    // 7. Vong lap gui tin nhan
    while (running) {
        printf("[Server]: ");
        fflush(stdout);

        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        if (!running) break;

        send(client_sock, buffer, strlen(buffer), 0);

        // Kiem tra lenh thoat
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
    close(client_sock);
    close(server_fd);
    printf("\n[Server] Da dong ket noi.\n");
    return 0;
}
