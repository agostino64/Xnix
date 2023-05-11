#include <stdio.h>
#include <ctype.h>
#include <libstr.h>
#include <xnixstd.h>
#include <drivers/kbd.h>

#include <arch/i686/vga_text.h>

// Función que muestra el mensaje de ayuda
void print_help() {
  printf("This system is for x86 (32bits)\n\n");
  printf("Write: \'version\' for version\n");
  printf("Write: \'xnix\' for ascii art\n");
  printf("Write: \'clear\' for clean\n");
  printf("Write: \'sleep\' for Zzz XnixOS\n");
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
"          .....::.::--: -........:.....           \n";

// Función que ejecuta los comandos ingresados
void commands(char *k_input) {
  if (k_strcmp(k_input, "help") == 0) {
    print_help();
  } else if (k_strcmp(k_input, "version") == 0) {
    printf("XnixOS version 0.1.1 (GCC %d.%d.%d)\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  } else if (k_strcmp(k_input, "clear") == 0) {
    VGA_clrscr();
  } else if (k_strcmp(k_input, "xnix") == 0) {
    printf("%s\n", ascii_xnix);
  } else if (k_strcmp(k_input, "sleep") == 0) {
    char time[100];
    printf("Time to sleep (milliseconds): ");
    kbd_init(time, sizeof(time));
    // Verificar si el valor ingresado es un número válido
    int is_valid = 1;
    for (size_t i = 0; time[i] != '\0'; i++)
    {
        if (!isdigit(time[i]))
        {
            is_valid = 0;
            break;
        }
    }
    if (is_valid)
    {
        int sleep_time = atoi(time);
        ksleep(sleep_time);
        printf("\n\nTotal time: %d\n", sleep_time);
    }
    else
      printf("Invalid number!\n");
  } else {
    printf("%s: Command not found.\n", k_input);
    printf("Input \'help\'\n");
  }
}

