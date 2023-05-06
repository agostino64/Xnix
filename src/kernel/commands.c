#include "stdio.h"
#include "string.h"

#include <arch/i686/vga_text.h>

// Función que muestra el mensaje de ayuda
void print_help() {
  printf("This system is for x86 (32bits)\n\n");
  printf("Write: \'version\' for version\n");
  printf("Write: \'refresh\' for restart OS\n");
  printf("Write: \'clear\' for clean\n");
}

// Función que ejecuta los comandos ingresados
void commands(char *k_input) {
  if (k_strcmp(k_input, "help") == 0) {
    print_help();
  } else if (k_strcmp(k_input, "version") == 0) {
    printf("tOS version 0.1 (GCC %d.%d.%d)\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  } else if (k_strcmp(k_input, "clear") == 0) {
    VGA_clrscr();
  } else if (k_strcmp(k_input, "refresh") == 0) {
    VGA_clrscr();
    jumpkernel();
  } else {
    printf("%s: Command not found.\n", k_input);
    printf("Input \'help\'\n");
  }
}

