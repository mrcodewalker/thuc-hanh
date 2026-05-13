#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s [-l | -a | -la]\n", argv[0]);
        return -1;
    }

    // -l
    if (strcmp(argv[1], "-l") == 0)
    {
        printf("Display full property\n");
    }

    // -a
    else if (strcmp(argv[1], "-a") == 0)
    {
        printf("Display hidden files\n");
    }

    // -la
    else if (strcmp(argv[1], "-la") == 0 ||
             strcmp(argv[1], "-al") == 0)
    {
        printf("Display full property\n");
        printf("Display hidden files\n");
    }

    else
    {
        printf("Invalid option!\n");
        return -1;
    }

    return 0;
}
