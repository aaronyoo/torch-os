#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>

#define MAX_PROCESS_NAME_LENGTH 20

// TODO: Possibly add back in later but right now we don't need
//       to really save any registers.
// typedef struct {
//     uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip;
// } registers_t;

typedef struct _process_t {
    uint32_t pid;
    char task_name[MAX_PROCESS_NAME_LENGTH];

    uint32_t* kernel_stack_top;
    uint32_t page_directory;
    struct _process_t* next_task;
    uint32_t state;
} process_t;

#endif