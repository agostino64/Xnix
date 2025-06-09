#ifndef SHELL_H
#define SHELL_H

// Command function signature with argument
typedef void (*cmd_func_t)(char *args);

// Shell command entry
typedef struct {
    const char *cmd;
    cmd_func_t func;
} Command;

void init_shell(void);

#endif
