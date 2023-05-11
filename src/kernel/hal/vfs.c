#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <drivers/kbd.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDOUT:
        for (size_t i = 0; i < size; i++)
            VGA_putc(data[i]);
        return size;

    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
            e9_putc(data[i]);
        return size;

    default:
        return -1;
    }
}

int VFS_Read(fd_t file, char *data, size_t size)
{
    if (file == VFS_FD_STDIN)
    {
      kbd_interrupt(data);
      
      return 1;
    }

    return 0; // archivo no válido
}
