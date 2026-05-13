#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd;

    if (argc != 2)
    {
        printf("Usage: %s <file_path>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1],
              O_WRONLY | O_CREAT | O_TRUNC,
              0644);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    write(fd, "Hello World\n", 12);

    close(fd);

    printf("[CHILD] Write file success.\n");

    return 0;
}
