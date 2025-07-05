#ifndef SHELL_H
#define SHELL_H

// Command function signature with argument
typedef void (*cmd_func_t)(char *args);

// Shell command entry
typedef struct {
    cmd_func_t func;         // Pointer to the command function
    const char *alias;       // Command alias
    const char *help;        // Short description
    const char *usage;       // Usage string
} Command;

void init_shell(void);

#endif
