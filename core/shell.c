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
#include <xnix/task.h>
#include <xnix/string.h>
#include "../build_info.h"

#define XNIX_VERSION "1.1.2-3"
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

#ifndef BUILD_OS
    #define BUILD_OS "Unknown"
#endif

#ifndef BUILD_USER
    #define BUILD_USER "unknown"
#endif


#define INITIAL_SIZE 10

// Shell buffer and state
static char *cmd = NULL;
static char *dir = NULL;
static uint32_t current_size = INITIAL_SIZE;

// External CPU check
extern int _cpuid_support(void);

// ================== Command Functions ==================

void help_func(char *args)
{
    printk("Xnix %s\n\n", XNIX_VERSION);
    printk("Commands:\n");
    printk(" version   Show version\n");
    printk(" clear     Clear screen\n");
    printk(" reboot    Reboot system\n");
    printk(" shutdown  Power off system\n");
    printk(" cpuinfo   Display CPU info\n");
    printk(" xnix      Show ASCII logo\n");
    printk(" mem       Show memory info\n");
    printk(" ls        List files in RAMFS\n");
    printk(" cat       Read files from RAMFS\n");
    printk("\nCopyright (C) 2023, 2025 Agustin Gutierrez (agostino64)\n");
}

void version_func(char *args)
{
    #ifdef DEBUG
        #define IS_DEBUG 1
    #else
        #define IS_DEBUG 0
    #endif
    
    const char *build_number = BUILD_NUM;

    /* Print version, build mode, compiler, date/time */
    printk("xnix %s-%s #%s %s %s (%s %d.%d.%d - %s@%s)\n",
        XNIX_VERSION,
        IS_DEBUG ? "debug" : "release",
        build_number,
        BUILD_TIME,
        BUILD_DATE,
        BUILD_COMPILER,
        BUILD_COMPILER_MAJOR,
        BUILD_COMPILER_MINOR,
        BUILD_COMPILER_PATCH,
        BUILD_USER,
	BUILD_OS);
}


void cpuinfo_func(char *args)
{
    if (_cpuid_support())
        detect_cpu();
    else
        printk("cpuid extension is not supported by the CPU.\n");
}

void meminfo_func(char *args)
{
    printk("Kernel memory usage: %u bytes / %u kB\n", get_memory_usage(), (get_memory_usage()/1024));
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
        if (strcmp(cmd_name, (char *)cmds[i].cmd) == 0)
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
    Command cmds[] = {
        { "help", help_func },
        { "clear", (cmd_func_t)clear_screen },
        { "version", version_func },
        { "reboot", (cmd_func_t)reboot },
        { "shutdown", (cmd_func_t)shutdown },
        { "cpuinfo", cpuinfo_func },
        { "mem", meminfo_func },
        { "ls", list_fs },
        { "cat", cat_fs }
    };

    if (cmd != NULL)
        exec_cmd(cmd, cmds, sizeof(cmds) / sizeof(cmds[0]));
}

/**
 * Shell main task function.
 * Runs as a cooperative task and yields after each command.
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
    while (1) {
        printk("%s:> ", dir);

        // Read line (blocks internally)
        gets();
        char *input = get_input_buffer();
        uint32_t input_len = strlen(input) + 1;

        // Resize cmd buffer if needed
        if (input_len > current_size) {
            char *new_cmd = (char*)krealloc(cmd, current_size, input_len);
            if (new_cmd) {
                cmd = new_cmd;
                current_size = input_len;
            } else {
                KLOG(LOG_LEVEL_ERROR, "[Shell] Could not expand cmd buffer\n");
                yield();
                continue;
            }
        }

        // Copy and execute
        strcpy(cmd, input);
        if (cmd[0] != '\0') {
            cmd_init();
        }

        // Yield to allow other tasks
        yield();
    }
}




