#include <xnix/userspace/xnix_stdio.h>

void entry_func(char *args) {
    printf("Hello, World!\n");
    printf("\nPlease write your name: ");
    char *name = scanf();
    printf("\nHello there %s!\n", name);
}
