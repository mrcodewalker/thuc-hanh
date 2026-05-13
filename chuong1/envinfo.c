#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char cwd[1024];

    // Lấy biến môi trường HOME
    char *home = getenv("HOME");

    // Lấy biến USER
    char *user = getenv("USER");

    // Lấy thư mục hiện tại
    getcwd(cwd, sizeof(cwd));

    printf("HOME Directory : %s\n", home);
    printf("User Name      : %s\n", user);
    printf("Current Path   : %s\n", cwd);

    return 0;
}
