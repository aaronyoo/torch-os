#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>

#define MAX_PROCESS_NAME_LENGTH 20

// Definitions for process states
#define RUNNING 0
#define READY_TO_RUN 1
#define BLOCKED 2

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
    uint32_t time_used;
} process_t;

#endif