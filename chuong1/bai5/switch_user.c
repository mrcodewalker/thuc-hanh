#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>

int main()
{
    struct passwd *pw;
    int fd;

    char filepath[256];

    printf("=====================================\n");
    printf(" BEFORE setuid()\n");
    printf("=====================================\n");

    printf("Current UID : %d\n", getuid());
    printf("Current GID : %d\n", getgid());

    // Lấy thông tin user B
    pw = getpwnam("testuser");

    if (pw == NULL)
    {
        perror("getpwnam");
        return -1;
    }

    printf("\nSwitching to user: %s\n", pw->pw_name);

    // Đổi group trước
    if (setgid(pw->pw_gid) != 0)
    {
        perror("setgid");
        return -1;
    }

    // Đổi user
    if (setuid(pw->pw_uid) != 0)
    {
        perror("setuid");
        return -1;
    }

    printf("\n=====================================\n");
    printf(" AFTER setuid()\n");
    printf("=====================================\n");

    printf("Current UID : %d\n", getuid());
    printf("Current GID : %d\n", getgid());

    // Tạo đường dẫn file trong HOME của testuser
    snprintf(filepath,
             sizeof(filepath),
             "/home/%s/userB_file.txt",
             pw->pw_name);

    printf("\nCreating file: %s\n", filepath);

    // Tạo file
    fd = open(filepath,
              O_WRONLY | O_CREAT | O_TRUNC,
              0644);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    // Ghi nội dung vào file
    write(fd,
          "This file belongs to user B\n",
          29);

    // Đóng file descriptor
    close(fd);

    printf("\n[OK] File created successfully.\n");

    return 0;
}
