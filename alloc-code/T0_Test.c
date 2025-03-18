#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    // print the process ID (PID) for tracking
    printf("Process running. PID: %d\n", getpid());
    printf("Press Enter to exit...\n");

    // keep the program running until the user presses Enter
    getchar();

    return 0;
}
