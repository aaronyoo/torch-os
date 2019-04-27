#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>

#define MAX_PROCESS_NAME_LENGTH 25

typedef struct process {
    uint32_t pid;
    char task_name[MAX_PROCESS_NAME_LENGTH];

    uint32_t kernel_stack_top;
    uint32_t page_directory;
    uint32_t* next_task;
    uint32_t state;
} process_t;

#endif