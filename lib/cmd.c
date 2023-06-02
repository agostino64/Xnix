#include <screen.h>
#include <libstr.h>
#include <kbd.h>
#include <cmd.h>

#include <version.h>

void start_console(char *input)
{  
    puts(">> ", 0x8);
    kbd_init(input, BUFFER_SIZE);
    if (k_strcmp(input, "") == 0)
      start_console(input);
}

void exec_cmd(char *input, Cmd *cmds, int num_cmds)
{
    for (int i = 0; i < num_cmds; i++) {
        if (k_strcmp(input, cmds[i].cmd) == 0) {
            cmds[i].func();
            return;
        }
    }

    printk("%s: Command not found.\n", input);
}

void cmd_init(char *input)
{
  Cmd cmds[] = {
    { "help", print_help },
    { "version", print_version },
    { "xnix", print_acsii },
    { "clear", kclear_screen }
  };
  
  exec_cmd(input, cmds, sizeof(cmds) / sizeof(cmds[0]));
}

void print_help()
{
  printk("This system is for x86 (32bits)\n\n");
  printk("Write: \'version\' for version\n");
  printk("Write: \'xnix\' for ascii art\n");
  printk("Write: \'clear\' for clean\n");
}

void print_version()
{
  printk("XnixOS version %s (GCC %d.%d.%d) %s %s\n", XNIX_VERSION, BUILD_GCC, BUILD_GCC_VERSION, BUILD_GCC_PATCH, BUILD_DATE, BUILD_TIME);
}

void print_acsii()
{
  char *ascii_xnix =
  "          ..        .  ..                         \n"
  "        :.:  :       .  :                         \n"
  "       ..:  ..    .  .. :.                        \n"
  "       .=:.. .     .  : ::                        \n"
  "       . .  ..      . ....                        \n"
  "       . .  : :  : ...                            \n"
  "        ....: - .. .                              \n"
  "         ::-:-..-:..                              \n"
  "         .  :-  .. .                              \n"
  "           ::  .-.  .                             \n"
  "           . .    .  .                            \n"
  "          .          .:.                          \n"
  "         .    .  :  .:-                           \n"
  "        -       :                                 \n"
  "        :     : :    ... .........                \n"
  "       :.     . .:..         .    :..             \n"
  "       :      .  :         .          ...         \n"
  "       =        .       ::               ..       \n"
  "       ..     :.     .  :                .:       \n"
  "        :.   :      .    ..              :-       \n"
  "          . .    .:.         ...        ..        \n"
  "          ..  .-=. .     .:=...--:...: ..         \n"
  "          .....::.::--: -........:.....           \n";

  printk("%s\n", ascii_xnix);
}
