// initrd.c -- Defines the interface for and structures relating to the initial ramdisk.
//             Written for JamesM's kernel development tutorials.

#include <stdint.h>
#include <xnix/initrd.h>
#include <xnix/common.h>
#include <xnix/heap.h>
#include <xnix/log.h>
#include <xnix/fs.h>  // fs_node_t
#include <xnix/log.h>  // fs_node_t
#include <xnix/vga.h>  // fs_node_t
#include <xnix/string.h>
#include <xnix/memory.h>

initrd_header_t *initrd_header;     // The header.
initrd_file_header_t *file_headers; // The list of file headers.
fs_node_t *initrd_root;             // Our root directory node.
fs_node_t *initrd_dev;              // We also add a directory node for /dev, so we can mount devfs later on.
fs_node_t *root_nodes;              // List of file nodes.
int nroot_nodes;                    // Number of file nodes.

struct dirent dirent;

static char *buffer = NULL;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length)
        return 0;
    if (offset+size > header.length)
        size = header.length-offset;
    memcpy(buffer, (uint8_t*) (header.offset+offset), size);
    KLOG(LOG_LEVEL_DEBUG, "Reading %u bytes from initrd file '%s' at offset %u\n", size, node->name, offset);
    return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index)
{
    if (node == initrd_root && index == 0)
    {
      strcpy(dirent.name, "dev");
      dirent.name[3] = 0;
      dirent.ino = 0;
      return &dirent;
    }

    if (index-1 >= nroot_nodes)
        return 0;
    strcpy(dirent.name, root_nodes[index-1].name);
    dirent.name[strlen(root_nodes[index-1].name)] = 0;
    dirent.ino = root_nodes[index-1].inode;
    return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name)
{
    if (node == initrd_root &&
        !strcmp(name, "dev") )
        return initrd_dev;

    int i;
    for (i = 0; i < nroot_nodes; i++)
        if (!strcmp(name, root_nodes[i].name))
            return &root_nodes[i];
    return 0;
}

fs_node_t *initialise_initrd(uint32_t location)
{
    KLOG(LOG_LEVEL_INFO, "Initializing initrd at memory location 0x%x\n", location);
    // Initialise the main and file header pointers and populate the root directory.
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *) (location+sizeof(initrd_header_t));
    if (!initrd_header) {
        KLOG(LOG_LEVEL_ERROR, "Initrd header is null!\n");
        return 0;
    }
    
    KLOG(LOG_LEVEL_INFO, "Initrd contains %d files\n", initrd_header->nfiles);
    
    // Initialise the root directory.
    initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!initrd_root) {
        KLOG(LOG_LEVEL_ERROR, "Failed to allocate memory for initrd root\n");
        return 0;
    }
    strcpy(initrd_root->name, "initrd");
    initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
    initrd_root->flags = FS_DIRECTORY;
    initrd_root->read = 0;
    initrd_root->write = 0;
    initrd_root->open = 0;
    initrd_root->close = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr = 0;
    initrd_root->impl = 0;

    // Initialise the /dev directory (required!)
    initrd_dev = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!initrd_dev) {
        KLOG(LOG_LEVEL_ERROR, "Failed to allocate memory for /dev node\n");
        return 0;
    }
    strcpy(initrd_dev->name, "dev");
    initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
    initrd_dev->flags = FS_DIRECTORY;
    initrd_dev->read = 0;
    initrd_dev->write = 0;
    initrd_dev->open = 0;
    initrd_dev->close = 0;
    initrd_dev->readdir = &initrd_readdir;
    initrd_dev->finddir = &initrd_finddir;
    initrd_dev->ptr = 0;
    initrd_dev->impl = 0;

    root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * initrd_header->nfiles);
    if (!root_nodes) {
        KLOG(LOG_LEVEL_ERROR, "Failed to allocate memory for initrd file nodes\n");
        return 0;
    }
    nroot_nodes = initrd_header->nfiles;

    // For every file...
    int i;
    for (i = 0; i < initrd_header->nfiles; i++)
    {
        // Edit the file's header - currently it holds the file offset
        // relative to the start of the ramdisk. We want it relative to the start
        // of memory.
        file_headers[i].offset += location;
        // Create a new file node.
        strcpy(root_nodes[i].name, (const char *)&file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = FS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].impl = 0;
        KLOG(LOG_LEVEL_DEBUG, "Initrd file #%d: name='%s', offset=0x%x, size=%u\n", i, root_nodes[i].name, file_headers[i].offset, file_headers[i].length);
    }
    KLOG(LOG_LEVEL_INFO, "Initrd initialized successfully.\n");
    return initrd_root;
}

int list_initrd(void)  // section 8
{
    // list the contents of /
    int i = 0;
    struct dirent *node = 0;

    node = readdir_fs(fs_root, i);

    while (node != 0)
    {
        fs_node_t *fsnode = finddir_fs(fs_root, node->name);

        //if node not exist, then skipt
        if (!fsnode)
            continue;
        
        // Node is a directory
        if ((fsnode->flags & 0x7) == FS_FILE)
            printk("File - %s\n", node->name);
        else
            printk("Dir  - %s\n", node->name);

        i += 1;
        node = readdir_fs(fs_root, i);

    }

    return 0;
}

// Use heap for reduce memory usage
int read_initrd(char *file)
{
    #define INITIAL_SIZE 256 // Initial buffer size
    #define MAX_BUFFER_SIZE 1024 // Maximum buffer size to prevent excessive memory allocation
    int i = 0;
    struct dirent *node = 0;
    static uint32_t mem_size = 264;

    if (buffer == NULL)
        buffer = (char*)kmalloc(mem_size);

    KLOG(LOG_LEVEL_DEBUG, "heap started with %u of size.\n", mem_size);    

    while ((node = readdir_fs(fs_root, i)) != 0)
    {
        fs_node_t *fsnode = finddir_fs(fs_root, node->name);
        i++;

        //if node not exist, then skipt
        if (!fsnode)
            continue;

        // Skip directories
        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
            continue;

        // Match file name
        if (strcmp(node->name, file) == 0)
        {
            KLOG(LOG_LEVEL_DEBUG, "node->name and %s match.\n", file);

            uint32_t file_len = fsnode->length;
            if (file_len + 1 > mem_size) {
                // not expanding buffer beyond 1KB
                if (mem_size > MAX_BUFFER_SIZE) {
                    KLOG(LOG_LEVEL_ERROR, "ERROR: File '%s' is too large (%u bytes), cannot read into buffer\n", file, file_len);
                    kfree(buffer);
                    buffer = NULL;
                    mem_size = INITIAL_SIZE; // Reset buffer size
                    KLOG(LOG_LEVEL_DEBUG, "Buffer reset to %u bytes\n", mem_size);
                    return 1; // Avoid excessive memory allocation
                }

                char *new_buffer = (char*)krealloc(buffer, mem_size, file_len + 1);
                if (new_buffer) {
                    buffer = new_buffer;
                    mem_size = file_len + 1;
                } else {
                    KLOG(LOG_LEVEL_ERROR, "ERROR: Could not expand buffer\n");
                    return 1;
                }
            }

            uint32_t size = read_fs(fsnode, 0, mem_size, (uint8_t*)buffer);
            for (uint32_t j = 0; j < size; j++)
                put(buffer[j]);

            // Shrink buffer if it grew too much
            if (mem_size > INITIAL_SIZE * 2) {
                char *shrunk_buffer = (char*)krealloc(buffer, mem_size, 264);
                if (shrunk_buffer) {
                    buffer = shrunk_buffer;
                    mem_size = INITIAL_SIZE;
                    KLOG(LOG_LEVEL_DEBUG, "[initrd] Shrunk buffer to %s bytes\n", mem_size);
                }
            }

            return 0; // Success
        }
    }

    return 1; // File not found
}