/*
 * Bai 6.5: Chuong trinh Setup (Cai dat) viet bang ngon ngu C
 * Sao chep file binary, file thu vien, va file config vao he thong.
 * Yeu cau quyen root (sudo).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define SRC_SERVER "packet/bin/chat-server"
#define SRC_CLIENT "packet/bin/chat-client"
#define SRC_LIB    "packet/lib/libchat.so"
#define SRC_CONF   "packet/config/chat.conf"

#define DEST_SERVER "/usr/local/bin/chat-server"
#define DEST_CLIENT "/usr/local/bin/chat-client"
#define DEST_LIB    "/usr/local/lib/libchat.so"
#define DEST_CONF   "/etc/chat.conf"

/* Ham copy file thu cong bang C va dat quyen han */
int copy_file(const char *src_path, const char *dest_path, mode_t mode) {
    FILE *src = fopen(src_path, "rb");
    if (src == NULL) {
        fprintf(stderr, "[ERROR] Khong the mo file nguon '%s': %s\n", src_path, strerror(errno));
        return -1;
    }

    FILE *dest = fopen(dest_path, "wb");
    if (dest == NULL) {
        fprintf(stderr, "[ERROR] Khong the tao file dich '%s': %s\n", dest_path, strerror(errno));
        fclose(src);
        return -1;
    }

    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            fprintf(stderr, "[ERROR] Loi khi ghi file dich '%s'\n", dest_path);
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    fclose(src);
    fclose(dest);

    // Thiet lap phan quyen cho file dich
    if (chmod(dest_path, mode) == -1) {
        fprintf(stderr, "[ERROR] Khong the chmod '%s' ve %o: %s\n", dest_path, mode, strerror(errno));
        return -1;
    }

    printf("  [OK] Copied: %s -> %s (Perms: %o)\n", src_path, dest_path, mode);
    return 0;
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║    BAI 6.5 - CHUONG TRINH SETUP (C)          ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Kiem tra quyen root
    if (getuid() != 0) {
        fprintf(stderr, "[ERROR] Ban phai chay chuong trinh duoi quyen root (sudo ./setup)!\n\n");
        return 1;
    }

    printf("[i] Dang kiem tra goi cai dat packet/...\n");
    // Kiem tra thu muc packet da duoc build hay chua
    if (access(SRC_SERVER, F_OK) == -1 || access(SRC_LIB, F_OK) == -1 || access(SRC_CONF, F_OK) == -1) {
        fprintf(stderr, "[ERROR] Thieu file trong packet/! Hay chay 'make' truoc khi setup.\n\n");
        return 1;
    }

    printf("[i] Bat dau sao chep cac tep tin he thong...\n");

    // 2. Sao chep library (libchat.so -> /usr/local/lib/)
    if (copy_file(SRC_LIB, DEST_LIB, 0755) == -1) {
        fprintf(stderr, "[ERROR] Cai dat shared library that bai!\n");
        return 1;
    }

    // 3. Sao chep binaries (chat-server, chat-client -> /usr/local/bin/)
    if (copy_file(SRC_SERVER, DEST_SERVER, 0755) == -1) {
        fprintf(stderr, "[ERROR] Cai dat chat-server that bai!\n");
        return 1;
    }
    if (copy_file(SRC_CLIENT, DEST_CLIENT, 0755) == -1) {
        fprintf(stderr, "[ERROR] Cai dat chat-client that bai!\n");
        return 1;
    }

    // 4. Sao chep config file (chat.conf -> /etc/chat.conf)
    if (copy_file(SRC_CONF, DEST_CONF, 0644) == -1) {
        fprintf(stderr, "[ERROR] Cai dat file cau hinh that bai!\n");
        return 1;
    }

    // 5. Lam moi link lien ket thu vien cua nhan Linux
    printf("[i] Cap nhat bo nho dem library (ldconfig)...\n");
    if (system("ldconfig") == -1) {
        perror("[ERROR] ldconfig");
    }

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  ✓ CAI DAT THANH CONG CHUONG TRINH CHAT!     ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("Huong dan chay chuong trinh tu bat ky dau trong terminal:\n");
    printf("  - Terminal 1:  chat-server\n");
    printf("  - Terminal 2:  chat-client\n\n");
    printf("File cau hinh tai: %s\n", DEST_CONF);

    return 0;
}
