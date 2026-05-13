#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int global_var = 100;

int main()
{
    int fd;
    pid_t pid;

    // Mở file trước khi fork
    fd = open("fork_output.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    printf("Before fork()\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    // Child process
    if (pid == 0)
    {
        global_var += 50;

        printf("\n[CHILD PROCESS]\n");
        printf("PID        : %d\n", getpid());
        printf("Parent PID : %d\n", getppid());
        printf("Message    : I am child\n");
        printf("global_var : %d\n", global_var);

        write(fd, "Child writes to file\n", 21);

        sleep(30);
    }

    // Parent process
    else
    {
        global_var += 10;

        printf("\n[PARENT PROCESS]\n");
        printf("PID        : %d\n", getpid());
        printf("Child PID  : %d\n", pid);
        printf("Message    : I am parent\n");
        printf("global_var : %d\n", global_var);

        write(fd, "Parent writes to file\n", 22);

        sleep(30);
    }

    close(fd);

    return 0;
}
