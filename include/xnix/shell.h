#ifndef SHELL_H
#define SHELL_H

typedef struct {
    char *cmd;
    void (*func)();
} Cmd;

void init_tty(char *cmd);

#endif
