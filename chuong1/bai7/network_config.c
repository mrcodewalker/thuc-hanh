#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("=====================================\n");
    printf(" NETWORK CONFIGURATION PROGRAM\n");
    printf("=====================================\n");

    // DOWN interface
    printf("\n[1] Bringing interface DOWN...\n");

    system("sudo ip link set eth0 down");

    // Set IP
    printf("\n[2] Setting IP address...\n");

    system("sudo ip addr add 192.168.1.100/24 dev eth0");

    // UP interface
    printf("\n[3] Bringing interface UP...\n");

    system("sudo ip link set eth0 up");

    // Show result
    printf("\n[4] Current network info:\n\n");

    system("ip addr show eth0");

    printf("\n[OK] Network configuration completed.\n");

    return 0;
}
