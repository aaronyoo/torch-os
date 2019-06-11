#include <stddef.h>
#include <stdint.h>
#include <logger.h>
#include <filesystem.h>
#include <memory.h>
#include <panic.h>
#include <string.h>

#define DEFAULT_FILE_SIZE 256

fs_node_t* fs_construct_node(char* name, fs_node_type_t type);
uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t ramdisk_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t ramdisk_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer);

// Root of the filesystem
fs_node_t* root = NULL;

struct block {
    uint32_t size;
    char name[MAX_FILENAME_LENGTH];
};

uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read != NULL) {
        return node->read(node, offset, size, buffer);
    } else {
        panic("fs_read function pointer not set!");
    }
}

uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write != NULL) {
        return node->write(node, offset, size, buffer);
    } else {
        panic("fs_write function pointer not set!");
    }
}

void fs_add_dirent(fs_node_t* directory, fs_node_t* file) {
    logf("Adding file %s to directory %s\n", file->name, directory->name);

    // Check that the nodes are of the correct type
    if (directory->type != FS_DIRECTORY) {
        panic("Trying to add an entry to a node that this not a directory!");
    }
    if (file->type != FS_FILE) {
        panic("Trying to add an entry that is not a file!");
    }

    fs_dirent_t* current = directory->directory.head;
    if (current == NULL) {
        // There are no files in the directory
        
        // The code here is a bit tricky, first allocate a new
        // directory entry and then use the current pointer. Not
        // doing this will break insertion.
        directory->directory.head = kalloc(sizeof(fs_dirent_t));
        current = directory->directory.head;
        current->node = file;
        current->next = NULL;
    } else {
        // Move down the list to find an open spot
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = kalloc(sizeof(fs_dirent_t));
        current = current->next;
        current->node = file;
        current->next = NULL;
    }

    // Add to the directory size
    directory->directory.size += 1;
}

fs_node_t* fs_construct_node(char* name, fs_node_type_t type) {
    fs_node_t* node = kalloc(sizeof(fs_node_t));
    memset(node->name, 0, sizeof(node->name));
    strcpy(node->name, name);
    node->type = type;

    // Change this based on the filesystem
    node->read = ramdisk_read;
    node->write = ramdisk_write;

    if (type == FS_FILE) {
        node->file.buffer = kalloc(256);
        node->file.canonical_size = 0;
        node->file.allocated_size = 256;
    } else if (type == FS_DIRECTORY) {
        node->directory.head = NULL;
        node->directory.size = 0;
        node->directory.parent = NULL;
    }

    // TODO: check the other node types here

    return node;
}

void read_ramdisk(uint32_t initrd_start, uint32_t initrd_end) {
    uint32_t current = initrd_start;
    while (current < initrd_end) {
        struct block* b = (struct block*) current;
        char* contents = (char*) current + sizeof(struct block);
        logf("Size: %u\n", b->size);
        logf("Name: %s\n", b->name);
        logf("Contents: %s\n", (char *) contents);

        fs_node_t* temp = fs_construct_node(b->name, FS_FILE);
        logf("%u\n", temp->file.allocated_size);
        logf("%u\n", b->size);
        fs_write(temp, 0, b->size, contents);
        fs_add_dirent(root, temp);

        // increment the current pointer past the block and
        // the content of the file
        current += sizeof(struct block) + b->size;
    }
}

void init_filesystem(uint32_t initrd_start, uint32_t initrd_end) {
    root = fs_construct_node("root", FS_DIRECTORY);
    read_ramdisk(initrd_start, initrd_end);
    
    char test[100];

    logf("%x\n", root->directory.head);

    fs_dirent_t* current = root->directory.head;
    logf("%x\n", current);
    while (current != NULL) {
        fs_read(current->node, 0, current->node->file.canonical_size, test);
        logf("Filename: %s\n", current->node->name);
        logf("%s\n", test);
        logf("Size: %u\n", current->node->file.canonical_size);
        current = current->next;
    }
}

//-----------------------------------------------------------------------------
// RAMDISK Implementation
// 
// Technically this should be in a separate file but I am just going to leave
// it here for now for convenience
//-----------------------------------------------------------------------------

// Reads from a ramdisk node.
//      Caller must ensure that buffer is large enough to hold write of size
//      If offset + size > canonical_size then return error (read past end of file)
//      If successful, [buffer, buffer + size] contains the result of the read
uint32_t ramdisk_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Check that the node is actually a file
    if (node->type != FS_FILE) {
        panic("Trying to RAMDISK read from a node that is not a file!");
    }

    // Make sure we aren't being asked to read past the end of the file
    if (offset + size > node->file.canonical_size) {
        panic("Trying to RAMDISK read past the end of a file!");
    }

    // Read the data into the buffer
    memcpy(buffer, node->file.buffer + offset, size);

    // Return the number of bytes written
    return size;
}

uint32_t ramdisk_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Check that the node is actually a file
    if (node->type != FS_FILE) {
        panic("Trying to RAMDISK write to a node that is not a file!");
    }

    // Check that the offset if not greater than the file size
    if (offset > node->file.canonical_size) {
        panic("Trying to RAMDISK write to a node an an invalid offset");
    }

    if (offset + size > node->file.allocated_size) {
        // TODO: need to allocate more space but panic for now
        // probably need an implementation of realloc, which might
        // need an implementation of free but hopefully not we will see.
        panic("Uh-oh");
    }

    // Copy data and adjust canonical size
    memcpy(node->file.buffer + offset, buffer, size);
    node->file.canonical_size += size;

    // Return the number of bytes written
    return size;
}

