/*
 * Bai 4.3: Gui/Nhan file giua nha va cong ty
 * FILE CLIENT (NGUOI GUI) - chay tai "nha"
 *
 * Client doc file tu dia, gui den server.
 * Cach dung:  ./file_client <IP_SERVER> <duong_dan_file>
 * Vi du:      ./file_client 192.168.1.10 baocao.pdf
 *             ./file_client 127.0.0.1 test.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdint.h>
#include <libgen.h>

#define PORT 9100
#define BUFFER_SIZE 4096

/* Gui dung n byte qua socket */
ssize_t send_all(int sock, const void *buf, size_t n) {
    size_t total = 0;
    const char *p = (const char *)buf;
    while (total < n) {
        ssize_t s = send(sock, p + total, n - total, 0);
        if (s <= 0) return s;
        total += s;
    }
    return total;
}

int main(int argc, char *argv[]) {
    char server_ip[64];
    char filepath[512];
    struct sockaddr_in server_addr;

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 4.3 - FILE CLIENT (Nguoi gui)         ║\n");
    printf("║   Vi tri: \"NHA\"                              ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Lay tham so
    if (argc >= 2) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
    } else {
        printf("Nhap IP server (vi du 192.168.1.10): ");
        fgets(server_ip, sizeof(server_ip), stdin);
        server_ip[strcspn(server_ip, "\n")] = '\0';
    }

    if (argc >= 3) {
        strncpy(filepath, argv[2], sizeof(filepath) - 1);
    } else {
        printf("Nhap duong dan file can gui: ");
        fgets(filepath, sizeof(filepath), stdin);
        filepath[strcspn(filepath, "\n")] = '\0';
    }

    // 1. Mo file can gui
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("[ERROR] Khong the mo file");
        return 1;
    }

    // Lay kich thuoc file
    struct stat st;
    stat(filepath, &st);
    uint64_t file_size = st.st_size;

    // Lay ten file (bo duong dan)
    char filepath_copy[512];
    strcpy(filepath_copy, filepath);
    char *filename = basename(filepath_copy);
    uint64_t name_len = strlen(filename);

    printf("[Client] File: %s (%lu bytes)\n", filename, (unsigned long)file_size);

    // 2. Tao socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("[ERROR] socket"); fclose(fp); return 1; }

    // 3. Cau hinh server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("[ERROR] IP khong hop le: %s\n", server_ip);
        close(sock); fclose(fp); return 1;
    }

    // 4. Ket noi
    printf("[Client] Dang ket noi den %s:%d ...\n", server_ip, PORT);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] connect");
        close(sock); fclose(fp); return 1;
    }
    printf("[Client] ✓ Da ket noi!\n");

    // 5. Gui header: [name_len][filename][file_size]
    uint64_t name_len_be = htobe64(name_len);
    uint64_t file_size_be = htobe64(file_size);

    send_all(sock, &name_len_be, sizeof(name_len_be));
    send_all(sock, filename, name_len);
    send_all(sock, &file_size_be, sizeof(file_size_be));

    // 6. Gui noi dung file
    char buffer[BUFFER_SIZE];
    uint64_t sent = 0;
    size_t n;
    printf("[Client] Dang gui...\n");

    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (send_all(sock, buffer, n) <= 0) {
            printf("\n[ERROR] Loi khi gui du lieu.\n");
            break;
        }
        sent += n;
        int percent = (int)((sent * 100) / (file_size ? file_size : 1));
        printf("\r[Client] Tien trinh: %d%% (%lu/%lu bytes)",
               percent, (unsigned long)sent, (unsigned long)file_size);
        fflush(stdout);
    }
    printf("\n");

    fclose(fp);

    // 7. Cho xac nhan tu server
    char ack[16] = {0};
    int r = recv(sock, ack, sizeof(ack) - 1, 0);
    if (r > 0 && strcmp(ack, "OK") == 0) {
        printf("[Client] ✓ Server xac nhan da nhan file thanh cong!\n");
    } else {
        printf("[Client] ✗ Khong nhan duoc xac nhan tu server.\n");
    }

    close(sock);
    printf("[Client] Hoan tat.\n");
    return 0;
}
