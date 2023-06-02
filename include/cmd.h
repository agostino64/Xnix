typedef struct {
    char *cmd;
    void (*func)();
} Cmd;

void start_console(char *k_input);
void exec_cmd(char *input, Cmd *cmds, int num_cmds);
void cmd_init(char *input);

// commands
void print_help();
void print_version();
void print_acsii();
