/*
 * Bai 6.5: Chuong trinh Uninstall (Go cai dat) viet bang ngon ngu C
 * Su dung he thong goi unlink de xoa file khoi he thong.
 * Yeu cau quyen root (sudo).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define DEST_SERVER "/usr/local/bin/chat-server"
#define DEST_CLIENT "/usr/local/bin/chat-client"
#define DEST_LIB    "/usr/local/lib/libchat.so"
#define DEST_CONF   "/etc/chat.conf"

/* Ham an toan xoa file va in thong bao */
int remove_file(const char *path) {
    // Kiem tra file co ton tai khong truoc khi xoa
    if (access(path, F_OK) == -1) {
        printf("  [i] File '%s' khong ton tai trong he thong.\n", path);
        return 0;
    }

    // Goi he thong unlink() de xoa file
    if (unlink(path) == -1) {
        fprintf(stderr, "[ERROR] Khong the xoa file '%s': %s\n", path, strerror(errno));
        return -1;
    }

    printf("  [OK] Removed: %s\n", path);
    return 0;
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 6.5 - CHUONG TRINH UNINSTALL (C)       ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Kiem tra quyen root
    if (getuid() != 0) {
        fprintf(stderr, "[ERROR] Ban phai chay chuong trinh duoi quyen root (sudo ./uninstall)!\n\n");
        return 1;
    }

    printf("[i] Bat dau go bo cac file khoi he thong...\n");
    int status = 0;

    // 2. Xoa binaries
    status |= remove_file(DEST_SERVER);
    status |= remove_file(DEST_CLIENT);

    // 3. Xoa library
    status |= remove_file(DEST_LIB);

    // 4. Xoa config
    status |= remove_file(DEST_CONF);

    // 5. Cap nhat lai ldconfig de go bo cache cua libchat.so
    printf("[i] Cap nhat lai bo nho dem library (ldconfig)...\n");
    if (system("ldconfig") == -1) {
        perror("[ERROR] ldconfig");
    }

    if (status == 0) {
        printf("\n[OK] ✓ Go cai dat chuong trinh hoan toan thanh cong!\n");
    } else {
        printf("\n[!] Hoan thanh go cai dat voi mot so loi (xem chi tiet tren).\n");
    }

    return 0;
}
