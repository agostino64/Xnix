/*
 *  Xnix Shell Interface
 *
 *  Copyright (C) 2023, 2025  Agustin Gutierrez
 *
 *  Provides a simple interactive command-line interface
 *  with support for internal commands and dynamic input buffer resizing.
 */

#include <stdint.h>
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
#include <xnix/string.h>
#include "../userspace/showcase.h"

#define INITIAL_SIZE 256

// Shell buffer and state
static char *cmd = NULL;
static char *dir = NULL;
static uint32_t current_size = INITIAL_SIZE;

// External CPU check
extern int _cpuid_support(void);

/**
 * Command function pointer type.
 */
typedef void (*cmd_func_ptr)(char *args);

// Definitions on top
void help_func(char *args);
void version_func(char *args);
void cpuinfo_func(char *args);
void meminfo_func(char *args);
void list_fs(char *args);
void cat_fs(char *args);
void clear_screen_wrapper(char *args);
void reboot_wrapper(char *args);
void shutdown_wrapper(char *args);


/**
 * Structure for shell command registration.
 */
typedef struct {
    cmd_func_ptr func;      ///< Pointer to the command function
    const char *alias;     ///< Command alias
    const char *help;      ///< Short description of the command
    const char *usage;     ///< Usage information for the command
} shellfunction;

#define CMDENTRY(fptr, alias, help, usage) { fptr, alias, help, usage }

// Registering command functions (declared before help_func)
Command CMDs[] = {
    CMDENTRY(help_func,            "help",     "Shows command list", NULL),
    CMDENTRY(version_func,         "version",  "Show kernel version and banner", NULL),
    CMDENTRY(clear_screen_wrapper, "clear",    "Clears the screen", NULL),
    CMDENTRY(reboot_wrapper,       "reboot",   "Reboot the system", NULL),
    CMDENTRY(shutdown_wrapper,     "shutdown", "Power off the system", NULL),
    CMDENTRY(cpuinfo_func,         "cpuinfo",  "Display CPU information", NULL),
    CMDENTRY(meminfo_func,         "mem",      "Show memory usage info", NULL),
    CMDENTRY(list_fs,              "ls",       "List files in RAMFS", NULL),
    CMDENTRY(cat_fs,               "cat",      "Display contents of a file in RAMFS", "cat <filename>"),
    CMDENTRY(entry_func,           "showcase", "Show case of USERSPACE functions.", NULL)
};

// Wrappers for functions without arguments
void clear_screen_wrapper(char *args) { clear_screen(); }
void reboot_wrapper(char *args) { reboot(); }
void shutdown_wrapper(char *args) { shutdown(); }

// ================== Command Functions ==================
void help_func(char *args)
{
    if (!args || args[0] == '\0') {
        printk("\nList of commands:\n");
        for (int i = 0; i < (int)(sizeof(CMDs)/sizeof(Command)); i++) {
            if (CMDs[i].usage == NULL)
                printk("%d %s - %s\n", i, CMDs[i].alias, CMDs[i].help);
            else
                printk("%d %s - %s\n\tUsage: %s\n", i, CMDs[i].alias, CMDs[i].help, CMDs[i].usage);
        }
    } else {
        printk("Invalid option: \"%s\"\n", args);
    }
}

extern const char xnix_proc_banner[];

// ================== Version Function ==================
void version_func(char *args)
{
    printk("%s\n", xnix_proc_banner);
}

// ================== CPU and Memory Info Functions ==================
void cpuinfo_func(char *args)
{
    if (_cpuid_support())
        detect_cpu();
    else
        printk("cpuid extension is not supported by the CPU.\n");
}

// ================== Memory Info Function ==================
void meminfo_func(char *args)
{
    printk("Kernel memory usage: %u bytes / %u kB\n", get_memory_usage(), (get_memory_usage()/1024));
    printk("Shell buffer size: %u bytes\n", current_size);
}

// ================== File System Functions ==================
void list_fs(char *args)
{
    list_initrd();
}

void cat_fs(char *args)
{
    if (args == NULL || *args == '\0')
    {
        printk("usage: cat <filename>\n");
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
        if (strcmp(cmd_name, (char *)cmds[i].alias) == 0)
        {
            cmds[i].func(cmd_args);
            return;
        }
    }

    printk("%s: command not found\n", cmd_name);
}

/**
 * Initializes shell commands and processes the current input.
 */
void cmd_init(void)
{
    if (cmd != NULL)
        exec_cmd(cmd, CMDs, sizeof(CMDs) / sizeof(CMDs[0]));
}

/**
 * Shell main task function.
 * Runs as a preemptive task.
 */
void shell_task(void) {
    uint32_t dir_buf_size = INITIAL_SIZE;
    uint32_t idx;
    struct dirent *node;

    KLOG(LOG_LEVEL_INFO, "[Shell] starting shell_task\n");

    // Allocate buffers
    if (!cmd)
        cmd = (char*)kmalloc(current_size);
    if (!dir)
        dir = (char*)kmalloc(dir_buf_size);

    // Find first directory in fs_root
    idx = 0;
    node = readdir_fs(fs_root, idx);
    while (node) {
        fs_node_t *fsnode = finddir_fs(fs_root, node->name);
        if ((fsnode->flags & 0x7) == FS_DIRECTORY) {
            uint32_t len = strlen(node->name) + 1;
            if (len > dir_buf_size) {
                char *new_dir = (char*)krealloc(dir, dir_buf_size, len);
                if (new_dir) {
                    dir = new_dir;
                    dir_buf_size = len;
                } else {
                    KLOG(LOG_LEVEL_ERROR, "[Shell] Could not expand dir buffer\n");
                }
            }
            strcpy(dir, node->name);
            break;
        }
        idx++;
        node = readdir_fs(fs_root, idx);
    }

    // Main shell loop
    int prompted = 0;
    while (1) {
        char *input = get_input_buffer();
        if (!prompted) {
            //KLOG(LOG_LEVEL_DEBUG, "[Shell] Prompting for input in directory: %s\n", dir);
            printk("%s:> ", dir);
            prompted = 1;
        }
        gets();
        //KLOG(LOG_LEVEL_DEBUG, "[Shell] Input received: %s\n", input);
        input = get_input_buffer();
        uint32_t input_len = strlen(input) + 1;

        // Resize cmd buffer if needed
        if (input_len > current_size) {
            char *new_cmd = (char*)krealloc(cmd, current_size, input_len);
            if (new_cmd) {
                cmd = new_cmd;
                current_size = input_len;
            } else {
                KLOG(LOG_LEVEL_ERROR, "[Shell] Could not expand cmd buffer\n");
                continue;
            }
        }

        // Copy and execute
        strcpy(cmd, input);
        if (cmd[0] != '\0') {
            cmd_init();
            // Clears the input buffer for the next command
            cmd[0] = '\0';
            input[0] = '\0';
            prompted = 0; // <-- Allows you to display the prompt again

            // Shrink cmd buffer if it grew too much
            if (current_size > INITIAL_SIZE * 2) {
                char *shrunk_cmd = (char*)krealloc(cmd, current_size, INITIAL_SIZE);
                if (shrunk_cmd) {
                    cmd = shrunk_cmd;
                    current_size = INITIAL_SIZE;
                    KLOG(LOG_LEVEL_DEBUG, "[Shell] Shrunk cmd buffer to INITIAL_SIZE\n");
                }
            }
        }
    }
}




