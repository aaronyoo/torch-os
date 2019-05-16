#include <process.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>
#include <stddef.h>
#include <timer.h>

extern void switch_to_task_stub(process_t* other);

// Pointers for task control block linked list
process_t* previous_task;
process_t* current_task;
process_t* first = NULL;
process_t* last = NULL;

// Counter for time accounting
uint32_t last_time = 0;

uint32_t next_available_pid = 0; // only grows linearly upward

void update_time(void) {
    uint32_t current_time = read_timer();
    uint32_t elapsed = elapsed_time(last_time, current_time);
    last_time = current_time;
    current_task->time_used += elapsed;
}

void switch_to_task(process_t* other) {
    update_time();
    previous_task = current_task;
    current_task = other;
    switch_to_task_stub(other);
}

static void yield(void) {
    switch_to_task(current_task->next_task);
}

void task1(void) {
    while (1) {
        logf("Running task 1\n");
        yield();
    }
}

void task2(void) {
    while (1) {
        logf("Running task 2\n");
        yield();
    }
}

void add_task(process_t* proc) {
    if (first == NULL) {
        // There are no tasks added yet so add it in front
        first = proc;
        last = proc;
        return;
    }

    // Insert the task between last and first
    last->next_task = proc;
    proc->next_task = first;

    last = proc;
}

process_t* create_kernel_task(void (*start_eip)(void)) {
    process_t* task = halloc(sizeof(process_t));
    task->pid = next_available_pid++;
    strcpy(task->task_name, "name");
    uint32_t* stack_area = halloc(4096);
    task->kernel_stack_top = stack_area + 1023; // change later, have to worry about memory corruption
    logf("The address of the other task stack: %x\n", task->kernel_stack_top);
    task->page_directory = (uint32_t) &page_directory; // the new task will have the same page directory
    task->next_task = NULL;
    task->state = 0;

    // Make sure to place return eip on the top of the kernel stack
    *task->kernel_stack_top = (uint32_t) start_eip;
    task->kernel_stack_top -= 4; // decrement for ebp edi esi ebx

    return task;
}

void init_tasking(void) {
    logf("Printing the structure of the process_t struct:\n");
    logf("\tPid: %x\n", offsetof(process_t, pid));
    logf("\tTask Name: %x\n", offsetof(process_t, task_name));
    logf("\tKernel Stack Top: %x\n", offsetof(process_t, kernel_stack_top));
    logf("\tPage Directory: %x\n", offsetof(process_t, page_directory));
    logf("\tNext Task: %x\n", offsetof(process_t, next_task));
    logf("\tState %x\n", offsetof(process_t, state));

    // Build a process control block for the running process
    process_t* kernel_task = halloc(sizeof(process_t));
    kernel_task->pid = next_available_pid++;
    strcpy(kernel_task->task_name, "kernel");
    kernel_task->kernel_stack_top = 0; // change later
    kernel_task->page_directory = (uint32_t) &page_directory;
    kernel_task->next_task = NULL;
    kernel_task->state = 0;

    // This task is the current_taskent task
    current_task = kernel_task;

    // Add all the tasks to the scheduler
    add_task(kernel_task);
    add_task(create_kernel_task(task1));
    add_task(create_kernel_task(task2));

    while(1) {
        logf("Running task 0\n");
        logf("Ticks: %u\n", current_task->time_used);
        yield();
    }
}
