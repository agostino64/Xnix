/*
 *  XnixOS
 *
 *  xnix/core/shell.c
 */

#include <xnix/vga.h>
#include <xnix/common.h>
#include <xnix/keyb.h>
#include <xnix/shell.h>
#include <xnix/timer.h>

#include <xnix/version.h>

void start_shell(char *cmd)
{
  /*
    puts(">> ", 0x8);
    kbd_init(cmd, BUFFER_SIZE);
    if (k_strcmp(cmd, "") == 0)
      start_console(cmd);
  */

  while (1)
  {
    puts(">> ", 0x8);
    cmd = gets();
    if (strcmp(cmd, "") == 0)
      continue;
    else
      break;
  }
}

void help_func()
{
  printk("This system is for x86 (32bits)\n\n");
  printk("Write: \'version\' for version\n");
  printk("Write: \'xnix\' for ascii art\n");
  printk("Write: \'clear\' for clean\n");
}

void version_func()
{
  printk("XnixOS version %s (GCC %d.%d.%d) %s %s\n", XNIX_VERSION, BUILD_GCC, BUILD_GCC_VERSION, BUILD_GCC_PATCH, BUILD_DATE, BUILD_TIME);
}

void acsii_func()
{
  char *ascii_xnix =
" __  __      _       ___  ____        \n"
" \\ \\/ /_ __ (_)_  __/ _ \\/ ___|    \n"
"  \\  /|  _ \\| \\ \\/ / | | \\___ \\ \n"
"  /  \\| | | | |>  <| |_| |___) |     \n"
" /_/\\_\\_| |_|_/_/\\_\\\\___/|____/  \n";

  printk("%s\n", ascii_xnix);
}

void exec_cmd(char *cmd, Cmd *cmds, int num_cmds)
{
    for (int i = 0; i < num_cmds; i++) {
        if (strcmp(cmd, cmds[i].cmd) == 0) {
            cmds[i].func();
            return;
        }
    }

    printk("%s: Command not found.\n", cmd);
}

void cmd_init(char *cmd)
{
  Cmd cmds[] = {
    { "help", help_func },
    { "version", version_func },
    { "xnix", acsii_func },
    { "clear", clear_vga }
  };

  exec_cmd(cmd, cmds, sizeof(cmds) / sizeof(cmds[0]));
}

void init_tty(char *cmd)
{
    while(1)
    {
      start_shell(cmd);
      cmd_init(cmd);
    }
}
