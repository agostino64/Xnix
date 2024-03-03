int detect_cpu(void);
int do_intel(void);
int do_amd(void);
void printregs(int eax, int ebx, int ecx, int edx);

void sti(void);
void cli(void);
void halt(void);

void reboot(void);
void shutdown(void);
