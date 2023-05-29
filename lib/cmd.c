#include <screen.h>
#include <libstr.h>
#include <kbd.h>

// Función que muestra el mensaje de ayuda
void print_help() {
  kprint("This system is for x86 (32bits)\n\n");
  kprint("Write: \'version\' for version\n");
  kprint("Write: \'xnix\' for ascii art\n");
  kprint("Write: \'clear\' for clean\n");
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
    kprint("XnixOS version 2.0\n");
  } else if (k_strcmp(k_input, "clear") == 0) {
    kclear_screen();
  } else if (k_strcmp(k_input, "xnix") == 0) {
    kprint_color(ascii_xnix, 0x3);
  } else {
    kprint_color("Command not found.\n", 0x4);
    kprint("Input \'help\'\n");
  }
}
