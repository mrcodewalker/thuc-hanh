/*
 * Bai 4.2: Chat Client-Server qua mang LAN
 * SERVER - lang nghe tren TAT CA interface (0.0.0.0)
 * Ho tro NHIEU client cung luc (multi-client)
 *
 * Moi client duoc xu ly boi 1 thread rieng.
 * Tin nhan tu 1 client duoc broadcast den tat ca client khac.
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
#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS];
char client_names[MAX_CLIENTS][64];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Gui tin nhan den tat ca client (tru sender) */
void broadcast(const char *msg, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_sock) {
            send(client_sockets[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Xoa client khoi danh sach */
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == sock) {
            for (int j = i; j < num_clients - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
                strcpy(client_names[j], client_names[j + 1]);
            }
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Thread xu ly 1 client */
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char msg[BUFFER_SIZE + 128];
    char name[64];
    int n;

    // Nhan ten client (tin nhan dau tien)
    memset(name, 0, sizeof(name));
    n = recv(sock, name, sizeof(name) - 1, 0);
    if (n <= 0) { close(sock); return NULL; }
    name[strcspn(name, "\n")] = '\0';

    // Luu ten
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == sock) {
            strncpy(client_names[i], name, 63);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("[Server] '%s' da tham gia chat.\n", name);

    snprintf(msg, sizeof(msg), ">>> '%s' da tham gia phong chat <<<\n", name);
    broadcast(msg, sock);

    // Vong lap nhan tin nhan
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) break;

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "/quit") == 0) break;

        printf("[%s]: %s\n", name, buffer);

        snprintf(msg, sizeof(msg), "[%s]: %s\n", name, buffer);
        broadcast(msg, sock);
    }

    // Client roi di
    printf("[Server] '%s' da roi phong chat.\n", name);
    snprintf(msg, sizeof(msg), ">>> '%s' da roi phong chat <<<\n", name);
    broadcast(msg, sock);

    remove_client(sock);
    close(sock);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.2 - CHAT SERVER (LAN, Multi-client) ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Tao socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("[ERROR] socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Cau hinh: lang nghe tren TAT CA interface (0.0.0.0)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 - tat ca IP cua may
    server_addr.sin_port = htons(PORT);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] bind");
        close(server_fd);
        return 1;
    }

    // 4. Listen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("[ERROR] listen");
        close(server_fd);
        return 1;
    }

    printf("[Server] Dang lang nghe tren 0.0.0.0:%d\n", PORT);
    printf("[Server] Client trong mang LAN ket noi qua IP cua may nay.\n");
    printf("[Server] (Xem IP bang lenh: ip addr  hoac  hostname -I)\n");
    printf("[Server] Toi da %d client cung luc.\n\n", MAX_CLIENTS);

    // 5. Vong lap chap nhan client moi
    while (1) {
        int *new_sock = malloc(sizeof(int));
        *new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (*new_sock < 0) {
            perror("[ERROR] accept");
            free(new_sock);
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            char *full = "Server day, thu lai sau!\n";
            send(*new_sock, full, strlen(full), 0);
            close(*new_sock);
            free(new_sock);
            continue;
        }
        client_sockets[num_clients] = *new_sock;
        strcpy(client_names[num_clients], "???");
        num_clients++;
        pthread_mutex_unlock(&clients_mutex);

        printf("[Server] Ket noi moi tu %s:%d (tong: %d client)\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), num_clients);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, new_sock);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
