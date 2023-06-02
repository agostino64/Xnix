#include <screen.h>
#include <libstr.h>
#include <kbd.h>

#include <version.h>

// Función que muestra el mensaje de ayuda
void print_help() {
  printk("This system is for x86 (32bits)\n\n");
  printk("Write: \'version\' for version\n");
  printk("Write: \'xnix\' for ascii art\n");
  printk("Write: \'clear\' for clean\n");
}

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
"          .....::.::--: -........:.....           \n\n";

// Función que ejecuta los comandos ingresados
void cmd_init(char *k_input) {
  if (k_strcmp(k_input, "help") == 0) {
    print_help();
  } else if (k_strcmp(k_input, "version") == 0) {
    printk("XnixOS version %s (GCC %d.%d.%d) %s %s\n", XNIX_VERSION, BUILD_GCC, BUILD_GCC_VERSION, BUILD_GCC_PATCH, BUILD_DATE, BUILD_TIME);
  } else if (k_strcmp(k_input, "clear") == 0) {
    kclear_screen();
  } else if (k_strcmp(k_input, "xnix") == 0) {
    printk(ascii_xnix);
  } else {
    printk("%s: Command not found.\n", k_input);
    printk("Input \'help\'\n");
  }
}
