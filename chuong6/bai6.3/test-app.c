/*
 * Bai 6.3: Ung dung kiem tra Driver nhan Linux o User Space
 * Mo file thiet bi /dev/misc-module de doc/ghi du lieu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define DEVICE_PATH "/dev/misc-module"
#define BUF_SIZE 512

int main() {
    int fd;
    char read_buf[BUF_SIZE];
    char write_buf[] = "Xin chao tu User Space! Toi da ghi thanh cong vao Kernel Module Driver.";
    ssize_t bytes_read, bytes_written;

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   BAI 6.3 - TEST APP FOR KERNEL DRIVER       ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Mo file thiet bi
    printf("[1] Dang mo file thiet bi '%s'...\n", DEVICE_PATH);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("[ERROR] Khong the mo file thiet bi. Ban da chay lenh load driver chua? Co quyen doc/ghi chua?");
        printf("\nGoi y:\n  1. sudo insmod misc-module.ko\n  2. sudo chmod 666 %s\n\n", DEVICE_PATH);
        return 1;
    }
    printf("    ✓ Mo thiet bi thanh cong!\n\n");

    // 2. Doc thong diep mac dinh ban dau tu Driver
    printf("[2] Dang doc thong diep mac dinh tu Driver...\n");
    memset(read_buf, 0, sizeof(read_buf));
    bytes_read = read(fd, read_buf, sizeof(read_buf) - 1);
    if (bytes_read == -1) {
        perror("[ERROR] read");
        close(fd);
        return 1;
    }
    read_buf[bytes_read] = '\0';
    printf("    ✓ Du lieu nhan duoc: \"%s\"\n\n", read_buf);

    // 3. Ghi du lieu moi tu User Space vao Driver
    printf("[3] Dang ghi du lieu moi vao Driver...\n");
    printf("    -> Noi dung ghi: \"%s\"\n", write_buf);
    bytes_written = write(fd, write_buf, strlen(write_buf));
    if (bytes_written == -1) {
        perror("[ERROR] write");
        close(fd);
        return 1;
    }
    printf("    ✓ Da ghi thanh cong %zd bytes vao Driver!\n\n", bytes_written);

    // 4. Reset con tro ve vi tri 0 de doc lai du lieu vua ghi
    printf("[4] Reset con tro file ve 0 (lseek SEEK_SET) de doc lai du lieu...\n");
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("[ERROR] lseek");
        close(fd);
        return 1;
    }

    // 5. Doc lai de kiem chung
    printf("[5] Dang doc lai du lieu vua ghi...\n");
    memset(read_buf, 0, sizeof(read_buf));
    bytes_read = read(fd, read_buf, sizeof(read_buf) - 1);
    if (bytes_read == -1) {
        perror("[ERROR] read");
        close(fd);
        return 1;
    }
    read_buf[bytes_read] = '\0';
    printf("    ✓ Du lieu moi trong Driver: \"%s\"\n\n", read_buf);

    // Dong file descriptor
    close(fd);
    printf("[OK] Kiem thu hoàn tat, chuong trinh chay thanh cong.\n");
    return 0;
}
