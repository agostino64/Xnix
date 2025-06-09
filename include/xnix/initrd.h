// initrd.h -- Defines the interface for and structures relating to the initial ramdisk.
//             Written for JamesM's kernel development tutorials.

#ifndef INITRD_H
#define INITRD_H

#include "common.h"
#include "fs.h"

typedef struct
{
    u32 nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
    u8 magic;     // Magic number, for error checking.
    s8 name[128];  // Filename.
    u32 offset;   // Offset in the initrd that the file starts.
    u32 length;   // Length of the file.
} initrd_file_header_t;

// Initialises the initial ramdisk. It gets passed the address of the multiboot module,
// and returns a completed filesystem node.
fs_node_t *initialise_initrd(u32 location);
int list_initrd(void);
int read_initrd(char *file);

#endif
