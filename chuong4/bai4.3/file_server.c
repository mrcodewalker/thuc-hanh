/*
 * Bai 4.3: Gui/Nhan file giua nha va cong ty
 * FILE SERVER (NGUOI NHAN) - chay tai "cong ty"
 *
 * Server lang nghe, nhan file tu client va luu vao thu muc received/
 * Giao thuc don gian:
 *   [8 byte: do dai ten file][ten file][8 byte: kich thuoc file][noi dung file]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdint.h>

#define PORT 9100
#define BUFFER_SIZE 4096
#define RECV_DIR "received"

/* Doc dung n byte tu socket (xu ly truong hop recv tra ve it hon) */
ssize_t recv_all(int sock, void *buf, size_t n) {
    size_t total = 0;
    char *p = (char *)buf;
    while (total < n) {
        ssize_t r = recv(sock, p + total, n - total, 0);
        if (r <= 0) return r;
        total += r;
    }
    return total;
}

/* Nhan 1 file tu client */
int receive_file(int client_sock) {
    uint64_t name_len, file_size;

    // 1. Nhan do dai ten file
    if (recv_all(client_sock, &name_len, sizeof(name_len)) <= 0) return -1;
    name_len = be64toh(name_len);

    if (name_len == 0 || name_len > 255) {
        printf("[ERROR] Ten file khong hop le.\n");
        return -1;
    }

    // 2. Nhan ten file
    char filename[256] = {0};
    if (recv_all(client_sock, filename, name_len) <= 0) return -1;
    filename[name_len] = '\0';

    // 3. Nhan kich thuoc file
    if (recv_all(client_sock, &file_size, sizeof(file_size)) <= 0) return -1;
    file_size = be64toh(file_size);

    printf("[Server] Dang nhan file: '%s' (%lu bytes)\n",
           filename, (unsigned long)file_size);

    // 4. Tao file de luu
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", RECV_DIR, filename);
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        perror("[ERROR] Khong the tao file");
        return -1;
    }

    // 5. Nhan noi dung file
    char buffer[BUFFER_SIZE];
    uint64_t received = 0;
    while (received < file_size) {
        size_t to_recv = (file_size - received < BUFFER_SIZE)
                         ? (file_size - received) : BUFFER_SIZE;
        ssize_t n = recv(client_sock, buffer, to_recv, 0);
        if (n <= 0) break;
        fwrite(buffer, 1, n, fp);
        received += n;

        // Hien thi tien trinh
        int percent = (int)((received * 100) / file_size);
        printf("\r[Server] Tien trinh: %d%% (%lu/%lu bytes)",
               percent, (unsigned long)received, (unsigned long)file_size);
        fflush(stdout);
    }
    printf("\n");

    fclose(fp);

    if (received == file_size) {
        printf("[Server] ✓ Nhan file thanh cong: %s\n", filepath);
        // Gui xac nhan ve client
        char *ack = "OK";
        send(client_sock, ack, strlen(ack), 0);
        return 0;
    } else {
        printf("[Server] ✗ Nhan file that bai (thieu du lieu).\n");
        return -1;
    }
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.3 - FILE SERVER (Nguoi nhan)        ║\n");
    printf("║   Vi tri: \"CONG TY\"                          ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Tao thu muc luu file
    mkdir(RECV_DIR, 0755);

    // 1. Tao socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("[ERROR] socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind tren tat ca interface
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] bind");
        close(server_fd);
        return 1;
    }

    // 3. Listen
    if (listen(server_fd, 5) < 0) {
        perror("[ERROR] listen");
        close(server_fd);
        return 1;
    }

    printf("[Server] Lang nghe tren 0.0.0.0:%d\n", PORT);
    printf("[Server] File nhan duoc luu vao thu muc: %s/\n", RECV_DIR);
    printf("[Server] Dang cho nhan file... (Ctrl+C de dung)\n\n");

    // 4. Vong lap nhan file
    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("[ERROR] accept");
            continue;
        }

        printf("─────────────────────────────────────────────\n");
        printf("[Server] Ket noi tu %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        receive_file(client_sock);
        close(client_sock);

        printf("[Server] Dang cho file tiep theo...\n\n");
    }

    close(server_fd);
    return 0;
}
