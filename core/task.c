/*
 * Xnix
 *
 * Copyright (C) 2025  Agustin Gutierrez
 *
 * ---------------------------------------------------------------
 * This file implements a preemptive, timer-based multitasking
 * system for the Xnix kernel. A round-robin scheduler
 * is invoked by the timer interrupt to switch tasks.
 *
 * Cooperative yielding is supported via a software interrupt,
 * which unifies all context switches through the same scheduler.
 * ---------------------------------------------------------------
 */

#include <stdint.h>
#include <xnix/isr.h>
#include <xnix/task.h>
#include <xnix/vga.h>
#include <xnix/log.h>
#include <xnix/shell.h>
#include <xnix/paging.h>
#include <xnix/heap.h>

/// Defines the size of the stack allocated for each new task. 4KB is standard.
#define TASK_STACK_SIZE 4096

// Global state for the tasking system
static Task *runningTask;       ///< Global pointer to the currently executing task.
static Task mainTask;           ///< The main kernel task.
static Task shellTask;          ///< The secondary task for the shell.
static Task printTask;         ///< The secondary task for the serial port.
static int next_task_id = 1;    ///< ID for the next task to be created.

// External dependencies
extern void shell_task(void);               ///< External entry point for the shell task.
extern void print_task(void);

extern page_directory_t *kernel_directory;  ///< The kernel's page directory, from paging.c.

/**
 * @brief Core round-robin scheduler.
 *
 * Saves the state of the current task, selects the next task from the
 * circular list, and restores its state to perform a context switch. It also
 * handles switching the virtual address space.
 */
void schedule(registers_t *regs)
{
    // Exit if multitasking is not yet initialized.
    if (!runningTask) return;

    // Save the state of the interrupted task from the stack frame.
    runningTask->regs.esp = regs->esp;
    runningTask->regs.ebp = regs->ebp;
    runningTask->regs.eax = regs->eax;
    runningTask->regs.ebx = regs->ebx;
    runningTask->regs.ecx = regs->ecx;
    runningTask->regs.edx = regs->edx;
    runningTask->regs.esi = regs->esi;
    runningTask->regs.edi = regs->edi;
    runningTask->regs.eip = regs->eip;
    runningTask->regs.eflags = regs->eflags;
    
    // Select the next task to run.
    runningTask = runningTask->next;

    // Restore the state of the new task into the interrupt stack frame.
    regs->esp = runningTask->regs.esp;
    regs->ebp = runningTask->regs.ebp;
    regs->eax = runningTask->regs.eax;
    regs->ebx = runningTask->regs.ebx;
    regs->ecx = runningTask->regs.ecx;
    regs->edx = runningTask->regs.edx;
    regs->esi = runningTask->regs.esi;
    regs->edi = runningTask->regs.edi;
    regs->eip = runningTask->regs.eip;
    regs->eflags = runningTask->regs.eflags;
    regs->cs = 0x08; // Kernel Code Segment
    regs->ds = 0x10; // Kernel Data Segment

    // Switch to the new task's address space.
    switch_page_directory(runningTask->page_directory);
}

/**
 * @brief Voluntarily yields CPU control via a software interrupt.
 */
void yield(void)
{
    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Task %d yielding CPU.\n", runningTask ? runningTask->id : -1);
    __asm__ __volatile__ ("int $0x80");
}

/**
 * @brief Creates and initializes a new task.
 *
 * Allocates a new stack, sets up the initial register state (EIP, ESP, EFLAGS),
 * and inserts the task into the scheduler's circular linked list.
 *
 * @return 0 on success, -1 on failure.
 */
int create_task(Task *parent, Task *new_task, void (*entry)(void), page_directory_t *page_dir)
{
    if (!parent || !new_task || !entry || !page_dir) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] createTask received one or more null pointers.\n");
        return -1;
    }
    
    // Allocate a 4KB stack for the new task from the kernel heap.
    uint32_t stack = (uint32_t)kmalloc(TASK_STACK_SIZE);
    if (!stack) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] Failed to allocate stack for new task ID %d.\n", next_task_id);
        return -1;
    }

    new_task->id = next_task_id++;
    new_task->regs.eax = 0;
    new_task->regs.ebx = 0;
    new_task->regs.ecx = 0;
    new_task->regs.edx = 0;
    new_task->regs.esi = 0;
    new_task->regs.edi = 0;
    new_task->regs.eflags = 0x202; // Interrupts enabled flag.
    new_task->regs.eip = (uint32_t)entry;
    new_task->page_directory = page_dir;

    // ** BUG FIX **: Point ESP to the top of the allocated stack.
    // The original code allocated 'stack' and then leaked it by calling kmalloc again.
    // This now correctly uses the 'stack' pointer.
    new_task->regs.esp = stack + TASK_STACK_SIZE;
    new_task->regs.ebp = new_task->regs.esp;

    // Insert the new task into the circular list after the parent.
    new_task->next = parent->next;
    parent->next = new_task;

    KLOG(LOG_LEVEL_INFO,
         "[Tasking] Created task %d (entry=0x%X, stack_top=0x%X) with parent %d\n",
         new_task->id, new_task->regs.eip, new_task->regs.esp, parent->id);

    return 0;
}

/**
 * @brief Initializes the multitasking system.
 */
void init_tasking(void)
{
    __asm__ __volatile__ ("cli"); // Disable interrupts during initialization.
    KLOG(LOG_LEVEL_INFO, "[Tasking] Initializing multitasking...\n");

    // 1. Initialize the main kernel task.
    mainTask.id = next_task_id++;
    mainTask.page_directory = kernel_directory;
    mainTask.next = &mainTask; // It's the only task, so it points to itself.

    // 2. Create the shell task with its own address space.
    page_directory_t *shell_page_dir = clone_directory(kernel_directory);
    if (!shell_page_dir) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] Failed to clone page directory for shell task.\n");
        // In a real scenario, this would be a fatal kernel panic.
        return;
    }
    if (create_task(&mainTask, &shellTask, shell_task, shell_page_dir) != 0) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] Failed to create the shell task.\n");
        return;
    }
    
    // 3. Create the serial task with its own address space.
    page_directory_t *print_page_dir = clone_directory(kernel_directory);
    if (!print_page_dir) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] Failed to clone page directory for serial port task.\n");
        // In a real scenario, this would be a fatal kernel panic.
        return;
    }
    if (create_task(&mainTask, &printTask, print_task, print_page_dir) != 0) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] Failed to create the print task.\n");
        return;
    }

    // 4. Set the main task as the first running task.
    runningTask = &mainTask;
    KLOG(LOG_LEVEL_INFO, "[Tasking] System initialized. Starting with task ID %d.\n", runningTask->id);

    __asm__ __volatile__ ("sti"); // Re-enable interrupts to start scheduling.
}