#include <process.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>
#include <stddef.h>
#include <timer.h>
#include <tty.h>
#include <panic.h>

extern void switch_to_task_stub(process_t* other);

void switch_to_task(process_t* other);
static void schedule(void);

// Pointers for task control block linked list
process_t* previous_task;
process_t* current_task;

// The scheduler maintains several different queues
// to keep track of every type of 
typedef struct {
    process_t* head;
    process_t* tail;
} list_t;

list_t ready_to_run = { .head = NULL, .tail = NULL };
list_t blocked = { .head = NULL, .tail = NULL };
list_t waiting = { .head = NULL, .tail = NULL };
list_t terminated = { .head = NULL, .tail = NULL };

// Counter for time accounting
uint32_t last_time = 0;

// Watermark counter to assign pid sequentially
uint32_t next_available_pid = 0;

// Enqueues a task to the a specified queue
void enqueue_task(list_t* queue, process_t* proc) {
    if (queue->head == NULL) {
        // There are no tasks added yet so add it in front
        queue->head = proc;
        queue->tail = proc;
        return;
    }

    // Insert the task at the end of the list
    queue->tail->next_task = proc;
    proc->next_task = NULL;
    queue->tail = proc;
}

// Terminates the current task
void terminate_task(void) {
    // TODO: can free certain resources like any user memory
    // or other stuff like that. But you CANNOT free the kernel
    // memory because it is currently being used by the stack

    // Error if the kernel task is terminated
    if (current_task->pid == 0) {
        panic("Attempt to terminate the kernel task!");
    }

    process_t* next_task = current_task->next_task;
    ready_to_run.head = current_task->next_task;
    current_task->state = TERMINATED;
    enqueue_task(&terminated, current_task);

    switch_to_task(next_task);

    panic("Control has come back to a terminated task!");
}

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

static void schedule(void) {
    if (current_task->next_task != NULL) {
        // Move the current task to the back and set the state
        // to READY_TO_RUN
        process_t* next_task = current_task->next_task;

        ready_to_run.head = current_task->next_task;
        current_task->state = READY_TO_RUN;
        enqueue_task(&ready_to_run, current_task);

        switch_to_task(next_task);
    }
}

void task1(void) {
    int x = 10;
    while (x-- > 0) {
        terminal_print("Running task 1\n");
        schedule();
    }
    terminate_task();
}

void task2(void) {
    while (1) {
        logf("Running task 2\n");
        schedule();
    }
}

// Creates a kernel task given a name and function pointer
//      Having no function pointer signals that there is no
//      stack setup needed. NULL should really only be used
//      for the initial kernel thread.
process_t* create_kernel_task(char* name, void (*start_eip)(void)) {
    process_t* task = halloc(sizeof(process_t));
    task->pid = next_available_pid++;
    strcpy(task->task_name, "name");
    uint32_t* stack_area = halloc(4096);
    task->kernel_stack_top = stack_area + 1023; // change later, have to worry about memory corruption
    logf("The address of the other task stack: %x\n", task->kernel_stack_top);
    task->page_directory = (uint32_t) &page_directory; // the new task will have the same page directory
    task->next_task = NULL;
    task->state = 1; // Ready to run by default

    if (start_eip != NULL) {
        // Make sure to place return eip on the top of the kernel stack
        *task->kernel_stack_top = (uint32_t) start_eip;
        task->kernel_stack_top -= 4; // decrement for ebp edi esi ebx
    }

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
    process_t* kernel_task = create_kernel_task("kernel", NULL);

    // This task is the current task
    current_task = kernel_task;

    // Add all the tasks to the scheduler
    enqueue_task(&ready_to_run, kernel_task);
    enqueue_task(&ready_to_run, create_kernel_task("task1", task1));
    enqueue_task(&ready_to_run, create_kernel_task("task2", task2));

    while(1) {
        logf("Running task 0\n");
        logf("Ticks: %u\n", current_task->time_used);
        schedule();
    }
}
