#include <screen.h>
#include <libstr.h>
#include <kbd.h>
#include <shell.h>
#include <timer.h>

#include <version.h>

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
    if (k_strcmp(cmd, "") == 0)
      continue;
    else
      break;
  }
}

void exec_cmd(char *cmd, Cmd *cmds, int num_cmds)
{
    for (int i = 0; i < num_cmds; i++) {
        if (k_strcmp(cmd, cmds[i].cmd) == 0) {
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
    { "clear", kclear_screen }
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
