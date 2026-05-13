#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;

    // Kiểm tra số lượng tham số
    if (argc != 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    // Mở file ở chế độ chỉ đọc
    fd = open(argv[1], O_RDONLY);

    // Kiểm tra lỗi
    if (fd < 0)
    {
        printf("Open file failed!\n");
        return -1;
    }

    printf("Open file success!\n");

    // Đóng file
    close(fd);

    return 0;
}
