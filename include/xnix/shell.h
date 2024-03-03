typedef struct {
    const char *cmd;
    void (*func)(void);
} Cmd;

void init_shell(void);
