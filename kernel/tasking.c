#include <process.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>
#include <stddef.h>

extern switch_to_task(process_t* other);

process_t* prev;
process_t* curr;
process_t* other; // temporary
process_t* another;
process_t* tasks[20]; // currently only support 20 tasks
uint32_t next_available_pid = 0; // only grows linearly upward

void switch_to_task_wrapper(process_t* other) {
    prev = curr;
    curr = other;
    switch_to_task(other);
}

void task1() {
    while (1) {
        logf("Running task 1\n");
        switch_to_task_wrapper(another);
    }
}

void task2() {
    while (1) {
        logf("Running task 2\n");
        switch_to_task_wrapper(other);
    }
}

uint32_t create_kernel_task(uint32_t* start_eip) {
    process_t* task = halloc(sizeof(process_t));
    task->pid = next_available_pid++;
    strcpy(task->task_name, "name");
    uint32_t* stack_area = halloc(4096);
    task->kernel_stack_top = stack_area + 1023; // change later, have to worry about memory corruption
    logf("The address of the other task stack: %x\n", task->kernel_stack_top);
    task->page_directory = &page_directory; // the new task will have the same page directory
    task->next_task = NULL;
    task->state = 0;

    // Make sure to place return eip on the top of the kernel stack
    *task->kernel_stack_top = (uint32_t) start_eip;
    task->kernel_stack_top -= 4; // decrement for ebp edi esi ebx

    return (uint32_t) task;
}

void init_tasking() {
    logf("Printing the structure of the process_t struct:\n");
    logf("\tPid: %x\n", offsetof(process_t, pid));
    logf("\tTask Name: %x\n", offsetof(process_t, task_name));
    logf("\tKernel Stack Top: %x\n", offsetof(process_t, kernel_stack_top));
    logf("\tPage Directory: %x\n", offsetof(process_t, page_directory));
    logf("\tNext Task: %x\n", offsetof(process_t, next_task));
    logf("\tState %x\n", offsetof(process_t, state));

    memset(tasks, 0, sizeof(tasks));
    tasks[0] = curr; // kernel task is the first task

    /* Build a process control block for the running process */
    curr = halloc(sizeof(process_t));
    curr->pid = next_available_pid++;
    strcpy(curr->task_name, "kernel");
    curr->kernel_stack_top = 0; // change later
    curr->page_directory = &page_directory;
    curr->next_task = NULL;
    curr->state = 0;

    logf("Current pid: %x\n", curr->pid);

    // Create task1
    other = create_kernel_task((uint32_t*) task1);
    another = create_kernel_task((uint32_t*) task2);
    logf("The address of the other task structure is: %x\n", other);
    switch_to_task_wrapper(other);
}
