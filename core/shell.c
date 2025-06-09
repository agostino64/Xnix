/*
 *  Xnix Shell Interface
 *
 *  Copyright (C) 2023, 2025  Agustin Gutierrez
 *
 *  Provides a simple interactive command-line interface
 *  with support for internal commands and dynamic input buffer resizing.
 */

#include <xnix/common.h>
#include <xnix/vga.h>
#include <xnix/drivers/keyb.h>
#include <xnix/cpu.h>
#include <xnix/shell.h>
#include <xnix/drivers/timer.h>
#include <xnix/isr.h>
#include <xnix/heap.h>
#include <xnix/log.h>
#include <xnix/initrd.h>

#define XNIX_VERSION "1.1.2-2"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

#ifdef __clang__
    #define BUILD_COMPILER "Clang"
    #define BUILD_COMPILER_MAJOR __clang_major__
    #define BUILD_COMPILER_MINOR __clang_minor__
    #define BUILD_COMPILER_PATCH __clang_patchlevel__
#elif defined(__GNUC__)
    #define BUILD_COMPILER "GCC"
    #define BUILD_COMPILER_MAJOR __GNUC__
    #define BUILD_COMPILER_MINOR __GNUC_MINOR__
    #define BUILD_COMPILER_PATCH __GNUC_PATCHLEVEL__
#else
    #define BUILD_COMPILER "Unknown"
    #define BUILD_COMPILER_MAJOR 0
    #define BUILD_COMPILER_MINOR 0
    #define BUILD_COMPILER_PATCH 0
#endif

#define INITIAL_SIZE 10

// Shell buffer and state
static char *cmd = NULL;
static char *dir = NULL;
static u32 current_size = INITIAL_SIZE;

// External CPU check
extern int _cpuid_support(void);

// ================== Command Functions ==================

void help_func(char *args)
{
    printk("Xnix %s - i386\n\n", XNIX_VERSION);
    printk("Commands:\n");
    printk("  version     Show version\n");
    printk("  clear       Clear screen\n");
    printk("  reboot      Reboot system\n");
    printk("  shutdown    Power off system\n");
    printk("  cpuinfo     Display CPU info\n");
    printk("  xnix        Show ASCII logo\n");
    printk("  mem         Show memory info\n");
    printk("  ls          List files in RAMFS\n");
    printk("  cat         Read files from RAMFS\n");
    printk("\nCopyright (C) 2023, 2025 Agustin Gutierrez (agostino64)\n");
}

void version_func(char *args)
{
    printk("Xnix Version %s (%s %d.%d.%d) %s %s\n",
        XNIX_VERSION,
        BUILD_COMPILER,
        BUILD_COMPILER_MAJOR,
        BUILD_COMPILER_MINOR,
        BUILD_COMPILER_PATCH,
        BUILD_DATE,
        BUILD_TIME);
}

void cpuinfo_func(char *args)
{
    if (_cpuid_support())
        detect_cpu();
    else
        printk("cpuid extension is not supported by the CPU.\n");
}

void acsii_func(char *args)
{
    const char *ascii_xnix =
        " __  __      _       ___  ____        \n"
        " \\ \\/ /_ __ (_)_  __/ _ \\/ ___|    \n"
        "  \\  /|  _ \\| \\ \\/ / | | \\___ \\ \n"
        "  /  \\| | | | |>  <| |_| |___) |     \n"
        " /_/\\_\\_| |_|_/_/\\_\\\\___/|____/  \n";

    printk("%s\n", ascii_xnix);
}

void meminfo_func(char *args)
{
    printk("Kernel memory usage: %u bytes\n", get_memory_usage());
    printk("Shell buffer size: %u bytes\n", current_size);
}

void list_fs(char *args)
{
    list_initrd();
}

void cat_fs(char *args)
{
    if (args == NULL || *args == '\0')
    {
        printk("Usage: cat <filename>\n");
        return;
    }

    if (read_initrd(args) != 0)
    {
        printk("file %s not exist\n", args);
        return;
    }
}

// ================== Command Execution ==================

/**
 * Executes a shell command by name with optional arguments.
 */
void exec_cmd(const char *input, Command *cmds, int num_cmds)
{
    // Split input into command and arguments
    char *space = strchr(input, ' ');
    char *cmd_name = NULL;
    char *cmd_args = NULL;

    if (space)
    {
        *space = '\0';
        cmd_name = (char*)input;
        cmd_args = space + 1;
    }
    else
    {
        cmd_name = (char*)input;
        cmd_args = NULL;
    }

    for (int i = 0; i < num_cmds; i++)
    {
        if (strcmp(cmd_name, (char *)cmds[i].cmd) == 0)
        {
            cmds[i].func(cmd_args);
            return;
        }
    }

    printk("%s: Command not found.\n", cmd_name);
}

/**
 * Initializes shell commands and processes the current input.
 */
void cmd_init(void)
{
    Command cmds[] = {
        { "help", help_func },
        { "clear", (cmd_func_t)clear_screen },
        { "version", version_func },
        { "reboot", (cmd_func_t)reboot },
        { "shutdown", (cmd_func_t)shutdown },
        { "cpuinfo", cpuinfo_func },
        { "xnix", acsii_func },
        { "mem", meminfo_func },
        { "ls", list_fs },
        { "cat", cat_fs }
    };

    if (cmd != NULL)
        exec_cmd(cmd, cmds, sizeof(cmds) / sizeof(cmds[0]));
}

/**
 * Initializes the shell and starts the input loop.
 * Waits for user input, dynamically resizes the buffer if needed,
 * and invokes command handlers.
 */
void init_shell(void)
{
    int i = 0;
    struct dirent *node = 0;
    static u32 size = INITIAL_SIZE;
    
    if (cmd == NULL)
        cmd = (char*)kmalloc(current_size);
        
    if (dir == NULL)
        dir = (char*)kmalloc(size); // is initial size
   
    node = readdir_fs( fs_root, i );

    while (node != 0)
    {
        fs_node_t *fsnode = finddir_fs(fs_root, node->name );

        // Node is a directory
        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
        {
            u32 len = strlen(node->name) + 1;
            
            // Resize buffer if needed
            if (len > size)
            {
                char* new_dir = (char*)krealloc(dir, size, len);
                if (new_dir)
                {
                    dir = new_dir;
                    size = len;
                }
                else
                {
                    KLOG(LOG_LEVEL_ERROR, "ERROR: Could not expand buffer\n");
                    continue;
                }
            }
            KLOG(LOG_LEVEL_INFO, "[Shell] Founded directory: %s\n", node->name);
            strcpy(dir, node->name);
        }

        i += 1;
        node = readdir_fs(fs_root, i);

    }
    while (1)
    {     
        printk("%s@kernel:> ", dir);  // userspace still not implemented yet );
        gets();         // Wait for input (populates buffer2)
        char* input = get_input_buffer();
        u32 input_len = strlen(input) + 1;  // +1 for null terminator

        // Resize buffer if needed
        if (input_len > current_size)
        {
            char* new_cmd = (char*)krealloc(cmd, current_size, input_len);
            if (new_cmd)
            {
                cmd = new_cmd;
                current_size = input_len;
            }
            else
            {
                KLOG(LOG_LEVEL_ERROR, "ERROR: Could not expand buffer\n");
                continue;
            }
        }

        strcpy(cmd, input);

        if (cmd[0] == '\0')  // Empty command, ignore
            continue;
            
        cmd_init();  // Try to run the command
    }
}

