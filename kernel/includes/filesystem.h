#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#define MAX_FILENAME_LENGTH 128

struct _fs_node_t;
typedef struct _fs_node_t fs_node_t;

// Constrain the declarations for read, write, open, and close operations
typedef uint32_t (*fs_read_type_t)(fs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*fs_write_type_t)(fs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef void (*fs_open_type_t)(fs_node_t*);
typedef void (*fs_close_type_t)(fs_node_t*);

typedef enum {
    FS_FILE,
    FS_DIRECTORY
} fs_node_type_t;

typedef struct {
    uint8_t* buffer;
    uint32_t canonical_size;
    uint32_t allocated_size;
} fs_file_t;

struct _fs_dirent_t {
    fs_node_t* node;
    struct _fs_dirent_t* next;
};
typedef struct _fs_dirent_t fs_dirent_t;

typedef struct {
    // TODO: make a list [I need a linked list]
    fs_dirent_t* head;
    uint32_t size;
    fs_node_t* parent;
} fs_directory_t;

struct _fs_node_t {
    char name[MAX_FILENAME_LENGTH];
    fs_node_type_t type;

    // Use pointer to functions
    fs_read_type_t read;
    fs_write_type_t write;
    fs_open_type_t open;
    fs_close_type_t close;

    union {
        fs_file_t file;
        fs_directory_t directory;
    };
};

void init_filesystem(uint32_t initrd_start, uint32_t initrd_end);


#endif