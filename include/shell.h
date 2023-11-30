typedef struct {
    char *cmd;
    void (*func)();
} Cmd;

void start_console(char *cmd);
void exec_cmd(char *input, Cmd *cmds, int num_cmds);
void cmd_init(char *cmd);

// commands
void help_func();
void version_func();
void acsii_func();
void init_tty(char *input);
