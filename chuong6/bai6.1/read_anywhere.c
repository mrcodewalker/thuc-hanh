/*
 * Bai 6.1: Tu tao 1 file text va doc no tu 1 vi tri bat ky.
 * Su dung cac he thong goi (system calls): open, write, lseek, read, close.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FILE_NAME "sample.txt"
#define DEFAULT_OFFSET 14
#define DEFAULT_LENGTH 20

int main(int argc, char *argv[]) {
    int fd;
    ssize_t bytes_written, bytes_read;
    char write_buf[] = "Hello, this is Chapter 6 Exercise 6.1. We are practicing Linux File I/O system calls.";
    char *read_buf;
    off_t offset = DEFAULT_OFFSET;
    size_t length = DEFAULT_LENGTH;

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║      BAI 6.1 - LSEEK FILE I/O SYSTEM CALL    ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Doc tham so dong lenh neu co
    if (argc >= 2) {
        offset = atol(argv[1]);
    }
    if (argc >= 3) {
        length = atoi(argv[2]);
    }

    // 1. Tao file va ghi du lieu mau vao file
    printf("[1] Dang tao file '%s' va ghi du lieu mau...\n", FILE_NAME);
    fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("[ERROR] open to write");
        return 1;
    }

    bytes_written = write(fd, write_buf, strlen(write_buf));
    if (bytes_written == -1) {
        perror("[ERROR] write");
        close(fd);
        return 1;
    }
    printf("    ✓ Ghi thanh cong %zd bytes: \"%s\"\n\n", bytes_written, write_buf);
    close(fd);

    // 2. Mo lai file o che do read-only
    printf("[2] Dang mo lai file '%s' o che do Read-Only...\n", FILE_NAME);
    fd = open(FILE_NAME, O_RDONLY);
    if (fd == -1) {
        perror("[ERROR] open to read");
        return 1;
    }

    // Kiem tra gioi han offset
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (offset > file_size) {
        printf("[!] Cảnh báo: Offset %ld vuot qua kich thuoc file (%ld bytes). Dat lai offset = 0.\n", offset, file_size);
        offset = 0;
    }

    // 3. Su dung lseek de di chuyen con tro file den vi tri mong muon
    printf("[3] Dang di chuyen con tro file (lseek) den vi tri (offset): %ld...\n", offset);
    off_t new_pos = lseek(fd, offset, SEEK_SET);
    if (new_pos == -1) {
        perror("[ERROR] lseek");
        close(fd);
        return 1;
    }
    printf("    ✓ Con tro file hien tai o vi tri byte thu: %ld\n\n", new_pos);

    // 4. Doc du lieu tu vi tri do
    read_buf = (char *)malloc(length + 1);
    if (read_buf == NULL) {
        perror("[ERROR] malloc");
        close(fd);
        return 1;
    }
    memset(read_buf, 0, length + 1);

    printf("[4] Dang doc %zu bytes tu vi tri byte thu %ld...\n", length, offset);
    bytes_read = read(fd, read_buf, length);
    if (bytes_read == -1) {
        perror("[ERROR] read");
        free(read_buf);
        close(fd);
        return 1;
    }
    read_buf[bytes_read] = '\0'; // Dam bao chuoi ket thuc bang NULL

    printf("    ✓ Doc thanh cong %zd bytes: \"%s\"\n\n", bytes_read, read_buf);

    // Don dep
    free(read_buf);
    close(fd);

    printf("[OK] Hoan thanh chuong trinh Bai 6.1.\n");
    return 0;
}
