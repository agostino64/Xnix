/*
Usage:
    gcc -Wall generate_initrd.c -o generate_initrd.o
    ./generate_initrd.o srcFilePath srcFile1 srcFile2
*/

/*
Description:
[quote = http://www.jamesmolloy.co.uk/tutorial_html/8.-The%20VFS%20and%20the%20initrd.html]
    My format does not support subdirectories.
    It stores the number of files in the system as the first 4 bytes (sizeof(int))
    of the initrd file.
    That is followed by a set number (64) of header structures, giving the
    names, offsets and sizes of the files contained.
    The actual file data follows.
[/quote]
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NFILES     64
#define MAGIC          0xBF
#define FILE_NAME_SIZE 128

const char *imagePath = "initrd.img";
const char *srcFSPath;

struct initrd_header {
    unsigned char magic;
    char name[FILE_NAME_SIZE];
    unsigned int offset;
    unsigned int length;
};

int main(int argc, char **argv) {
    int i, sz_initrd_header, contentSize, headerSize, fileSize;
    char *dstFile;
    char srcFile[FILE_NAME_SIZE];
    srcFSPath = argv[1];
    int nHeaders = argc - 2;
    struct initrd_header headers[MAX_NFILES];
    sz_initrd_header = sizeof(struct initrd_header);
    unsigned int offset = sz_initrd_header * MAX_NFILES + sizeof(int);

    for (i = 0; i < MAX_NFILES; i++) {
        if (i < nHeaders) {
            dstFile = argv[2 + i];
            strcpy(srcFile, srcFSPath);
            strcat(srcFile, dstFile);
            printf("Writing file %s to %s at 0x%x\n", srcFile, dstFile, offset);
            strcpy(headers[i].name, dstFile);
            headers[i].offset = offset;
            FILE *stream = fopen(srcFile, "r");
            if (stream == 0) {
                printf("Error: file not found: %s\n", srcFile);
                return 1;
            }
            fseek(stream, 0, SEEK_END);
            headers[i].length = ftell(stream);
            fclose(stream);
            headers[i].magic = MAGIC;
            offset += headers[i].length;
        } else {
            memset(headers[i].name, 0, FILE_NAME_SIZE);
            headers[i].offset = 0;
            headers[i].length = 0;
            headers[i].magic = MAGIC;
        }
    }

    FILE *wstream = fopen(imagePath, "w");
    fwrite(&nHeaders, sizeof(int), 1, wstream);
    fwrite(headers, sz_initrd_header, MAX_NFILES, wstream);

    contentSize = 0;
    for (i = 0; i < nHeaders; i++) {
        dstFile = argv[2 + i];
        strcpy(srcFile, srcFSPath);
        strcat(srcFile, dstFile);
        FILE *stream = fopen(srcFile, "r");
        unsigned char *buf = (unsigned char *)malloc(headers[i].length);
        fread(buf, 1, headers[i].length, stream);
        fwrite(buf, 1, headers[i].length, wstream);
        fclose(stream);
        free(buf);
        contentSize += headers[i].length;
    }

    fclose(wstream);
    headerSize = sz_initrd_header * MAX_NFILES;
    fileSize = sizeof(int) + headerSize + contentSize;

    printf("Image is %d bytes long:\n", fileSize);
    printf("  nFiles        uses %lu bytes\n", sizeof(int));
    printf("  file headers  use  %d bytes\n", headerSize);
    printf("  file contents use  %d bytes\n", contentSize);

    return 0;
}

