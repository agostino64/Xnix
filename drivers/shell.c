/*
 *  Xnix
 *
 *  Copyright (C) 2022, 2023  Agustin Gutierrez
 */
 
#include <xnix/common.h>
#include <xnix/vga.h>
#include <xnix/keyb.h>
#include <xnix/cpu.h>
#include <xnix/shell.h>
#include <xnix/timer.h>
#include <xnix/isr.h>

#define XNIX_VERSION "0.1"
#define BUILD_GCC __GNUC__
#define BUILD_GCC_VERSION __GNUC_MINOR__
#define BUILD_GCC_PATCH __GNUC_PATCHLEVEL__
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

static char* cmd;
extern int _cpuid_support(void);

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
  printk(" error       test error\n");
  printk("\nCopyright (C) 2023, 2024 Agustin Gutierrez\n");
}

void version_func(void)
{
  printk("Xnix Version %s (GCC %d.%d.%d) %s %s\n", XNIX_VERSION, BUILD_GCC, BUILD_GCC_VERSION, BUILD_GCC_PATCH, BUILD_DATE, BUILD_TIME);
}

void cpuinfo_func(void)
{
  if (_cpuid_support())
    detect_cpu();
  else
    printk("cpuid extension is not supported by the CPU.\n");
}

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

void error_func(void)
{
  KERN_ERR("error test!\n");
}

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
        { "error", error_func }
    };

    exec_cmd(cmds, sizeof(cmds) / sizeof(cmds[0]));
}

void init_shell(void) {
init:
    write(">> ");
    cmd = gets();
    if (cmd[0] == '\0') // shell is empty
    	goto init;
    else
    {
        cmd_init();
        goto init;
    }
}
