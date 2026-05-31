/*
 * Bai 4.2: Chat Client-Server qua mang LAN
 * CLIENT - ket noi den server qua dia chi IP trong LAN
 *
 * Cach dung:  ./client <IP_SERVER> <TEN>
 * Vi du:      ./client 192.168.1.10 Alice
 *             ./client 127.0.0.1 Bob   (neu test cung may)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int sock = -1;
volatile int running = 1;
char my_name[64];

/* Thread nhan tin nhan tu server */
void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    int n;

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            printf("\n[!] Mat ket noi voi server.\n");
            running = 0;
            break;
        }
        printf("\r%s[%s]: ", buffer, my_name);
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];
    char server_ip[64];

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.2 - CHAT CLIENT (LAN)               ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Lay IP server va ten tu tham so dong lenh
    if (argc >= 2) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
    } else {
        printf("Nhap IP server (vi du 192.168.1.10, hoac 127.0.0.1): ");
        fgets(server_ip, sizeof(server_ip), stdin);
        server_ip[strcspn(server_ip, "\n")] = '\0';
    }

    if (argc >= 3) {
        strncpy(my_name, argv[2], sizeof(my_name) - 1);
    } else {
        printf("Nhap ten cua ban: ");
        fgets(my_name, sizeof(my_name), stdin);
        my_name[strcspn(my_name, "\n")] = '\0';
    }

    // 1. Tao socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("[ERROR] socket"); return 1; }

    // 2. Cau hinh dia chi server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("[ERROR] Dia chi IP khong hop le: %s\n", server_ip);
        close(sock);
        return 1;
    }

    // 3. Ket noi
    printf("\n[Client] Dang ket noi den %s:%d ...\n", server_ip, PORT);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] connect");
        close(sock);
        return 1;
    }

    printf("[Client] ✓ Da ket noi! Chao mung %s.\n", my_name);
    printf("[Client] Go tin nhan + Enter de gui. Go '/quit' de thoat.\n\n");

    // 4. Gui ten cho server (tin nhan dau tien)
    char name_msg[80];
    snprintf(name_msg, sizeof(name_msg), "%s\n", my_name);
    send(sock, name_msg, strlen(name_msg), 0);

    // 5. Tao thread nhan tin nhan
    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    // 6. Vong lap gui tin nhan
    while (running) {
        printf("[%s]: ", my_name);
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
    printf("\n[Client] Da thoat phong chat.\n");
    return 0;
}
