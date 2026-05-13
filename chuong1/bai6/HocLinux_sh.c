#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    // Kiểm tra tham số
    if (argc < 2)
    {
        printf("Usage: %s <script.sh> [argument]\n",
               argv[0]);
        return -1;
    }

    printf("=====================================\n");
    printf("         HocLinux_sh Shell\n");
    printf("=====================================\n");

    printf("\n[INFO] Script : %s\n", argv[1]);

    // Tạo process con
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    // Child process
    if (pid == 0)
    {
        printf("\n[CHILD] Running shell script...\n\n");

        // Có argument
        if (argc == 3)
        {
            execl("/bin/bash",
                  "bash",
                  argv[1],
                  argv[2],
                  NULL);
        }

        // Không có argument
        else
        {
            execl("/bin/bash",
                  "bash",
                  argv[1],
                  NULL);
        }

        perror("execl");
        exit(-1);
    }

    // Parent process
    else
    {
        wait(&status);

        printf("\n=====================================\n");
        printf("           SCRIPT FINISHED\n");
        printf("=====================================\n");

        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);

            printf("\n[PARENT] Exit code : %d\n",
                   exit_code);

            if (exit_code == 0)
            {
                printf("[PARENT] Script executed successfully.\n");
            }
            else
            {
                printf("[PARENT] Script execution failed.\n");
            }
        }
    }

    return 0;
}
