#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    printf("[PARENT] Starting parent process...\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    // Child process
    if (pid == 0)
    {
        printf("[CHILD] Executing child program...\n");

        execl("./child",
              "child",
              "output.txt",
              NULL);

        perror("execl");
        exit(-1);
    }

    // Parent process
    else
    {
        wait(&status);

        printf("\n[PARENT] Child process finished.\n");

        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);

            printf("[PARENT] Child exit code: %d\n",
                   exit_code);

            if (exit_code == 0)
            {
                printf("[PARENT] Child executed successfully.\n");
            }
            else
            {
                printf("[PARENT] Child execution failed.\n");
            }
        }
    }

    return 0;
}
