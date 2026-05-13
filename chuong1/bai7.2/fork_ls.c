#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    FILE *fp;
    char buffer[256];

    printf("=====================================\n");
    printf(" FORK + LS + FILE DEMO\n");
    printf("=====================================\n");

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    // Child process
    if (pid == 0)
    {
        printf("\n[CHILD] Running ls command...\n");

        int ret = system("ls -l > output.txt");

        if (ret != 0)
        {
            printf("[CHILD] ls command failed.\n");
            exit(-1);
        }

        printf("[CHILD] Output written to output.txt\n");

        exit(0);
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

            // Success
            if (exit_code == 0)
            {
                printf("\n=====================================\n");
                printf(" CONTENT OF output.txt\n");
                printf("=====================================\n\n");

                fp = fopen("output.txt", "r");

                if (fp == NULL)
                {
                    perror("fopen");
                    return -1;
                }

                while (fgets(buffer,
                             sizeof(buffer),
                             fp) != NULL)
                {
                    printf("%s", buffer);
                }

                fclose(fp);
            }
            else
            {
                printf("[PARENT] Child execution failed.\n");
            }
        }
    }

    return 0;
}
