#include <process.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>

process_t* curr;

void init_tasking() {
    /* Build a process control block for the running process */
    curr = halloc(sizeof(process_t));
    curr->pid = 0;
    strcpy(curr->task_name, "kernel");
    curr->kernel_stack_top = 0; // change later
    curr->page_directory = &page_directory;
    curr->next_task = NULL;
    curr->state = 0;

    logf("%x\n", curr->pid);
}

void create_new_task() {

}

void switch_to_task(process_t* other) {

}