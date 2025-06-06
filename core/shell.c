/*
 *  Xnix
 *
 *  Copyright (C) 2022, 2025  Agustin Gutierrez
 *
 *  Xnix Shell Interface
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

// Shell build and version info
#define XNIX_VERSION "0.1.2-1"
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

// Default input buffer size
#define INITIAL_SIZE 10

// Shell command buffer and size tracking
static char* cmd = NULL;
static u32 current_size = INITIAL_SIZE;

// External low-level function for CPU feature detection
extern int _cpuid_support(void);

/**
 * Prints the help message showing all available shell commands.
 */
void help_func(void)
{
    printk("Xnix %s - i386\n\n", XNIX_VERSION);
    printk("Option      meaning\n");
    printk(" version     get version\n");
    printk(" clear       clear screen\n");
    printk(" reboot      reboot system\n");
    printk(" shutdown    poweroff system\n");
    printk(" cpuinfo     print cpu info\n");
    printk(" xnix        print xnix logo\n");
    printk(" mem         shows the memory allocated for the shell\n");
    printk(" error       test error\n");
    printk("\nCopyright (C) 2022, 2025 Agustin Gutierrez\n");
}

/**
 * Prints the version and build metadata for the current kernel.
 */
void version_func(void)
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

/**
 * Displays CPU information using CPUID instruction, if supported.
 */
void cpuinfo_func(void)
{
    if (_cpuid_support())
        detect_cpu();
    else
        printk("cpuid extension is not supported by the CPU.\n");
}

/**
 * Executes a command if found in the command list.
 * If not found, prints an error message.
 *
 * @param cmds Array of Cmd structs.
 * @param num_cmds Number of entries in cmds.
 */
void exec_cmd(Cmd *cmds, int num_cmds)
{
    for (int i = 0; i < num_cmds; i++)
    {
        if (strcmp(cmd, (char*)cmds[i].cmd) == 0)
        {
            cmds[i].func();
            return;
        }
    }

    printk("%s: Command not found.\n", cmd);
}

/**
 * Prints the ASCII Xnix logo.
 */
void acsii_func(void)
{
    char *ascii_xnix = {
        " __  __      _       ___  ____        \n"
        " \\ \\/ /_ __ (_)_  __/ _ \\/ ___|    \n"
        "  \\  /|  _ \\| \\ \\/ / | | \\___ \\ \n"
        "  /  \\| | | | |>  <| |_| |___) |     \n"
        " /_/\\_\\_| |_|_/_/\\_\\\\___/|____/  \n"
    };

    printk("%s\n", ascii_xnix);
}

/**
 * Prints an error message for kernel log testing.
 */
void error_func(void)
{
    KLOG(LOG_LEVEL_ERROR, "error test!\n");
}

/**
 * Prints the currently allocated buffer size for the shell.
 */
void meminfo_func(void)
{
    printk("Shell buffer size: %u bytes\n", current_size);
}

/**
 * Initializes the shell command set and dispatches the current input.
 */
void cmd_init(void)
{
    Cmd cmds[] = {
        { "help", help_func },
        { "clear", clear_screen },
        { "version", version_func },
        { "reboot", reboot },
        { "shutdown", shutdown },
        { "cpuinfo", cpuinfo_func },
        { "xnix", acsii_func },
        { "mem", meminfo_func },
        { "error", error_func }
    };

    exec_cmd(cmds, sizeof(cmds) / sizeof(cmds[0]));
}

/**
 * Initializes the shell and starts the input loop.
 * Waits for user input, dynamically resizes the buffer if needed,
 * and invokes command handlers.
 */
void init_shell(void)
{
    if (cmd == NULL)
        cmd = (char*)kmalloc(current_size);
        
    while (1)
    {     
        write(">> ");  // Prompt
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


