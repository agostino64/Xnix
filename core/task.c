/*
 *  Xnix
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Base code taken from https://wiki.osdev.org/Cooperative_Multitasking
 *
 *  ---------------------------------------------------------------
 *  This file implements a basic cooperative multitasking system
 *  for the Xnix operating system. Tasks are represented by Task
 *  structures and are manually switched using the yield() function.
 *  
 *  The goal is to allow multiple execution contexts to share the CPU
 *  by explicitly yielding control. A circular linked list of tasks is
 *  used, and context switching is performed by saving/restoring CPU
 *  registers.
 *
 *  This initial implementation serves as a foundation for evolving
 *  into more advanced multitasking with scheduling (e.g., round-robin,
 *  priorities, etc.).
 *  ---------------------------------------------------------------
 */

#include <xnix/isr.h>
#include <xnix/task.h>
#include <xnix/vga.h>
#include <xnix/log.h>
#include <xnix/shell.h>

/// Global structure pointing to the currently running task
static Task *runningTask;

/// Main task (the kernel or initial process)
static Task mainTask;

/// Secondary test task (cooperative)
static Task otherTask;

/// External: allocates a page for the stack of a new task (defined elsewhere)
extern void* alloc_task_stack_page(void);

/**
 * Main function of the secondary task.
 * Prints test messages and continuously yields the CPU.
 */
static void otherMain(void) {
    KLOG(LOG_LEVEL_INFO, "[task] Hello multitasking world!\n");
    yield();
}

/**
 * @brief Initializes the cooperative multitasking subsystem.
 *
 * This function sets up the main task, creates a secondary task (otherMain),
 * and links both in a circular list. It then calls yield() to begin
 * context switching to the other task.
 */
void initTasking(void) {
    KLOG(LOG_LEVEL_INFO, "[Tasking] Initializing multitasking...\n");

    // Capture CR3 (current page directory) and flags (EFLAGS)
    __asm__ __volatile__ ("movl %%cr3, %%eax; movl %%eax, %0;\n" : "=m"(mainTask.regs.cr3) :: "%eax");
    __asm__ __volatile__ ("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;\n" : "=m"(mainTask.regs.eflags) :: "%eax");

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] mainTask CR3: 0x%X, EFLAGS: 0x%X\n", mainTask.regs.cr3, mainTask.regs.eflags);

    // Create secondary task
    createTask(&otherTask, otherMain, mainTask.regs.eflags, (u32*)mainTask.regs.cr3);

    // Link the tasks in a circular list
    mainTask.next = &otherTask;
    otherTask.next = &mainTask;

    runningTask = &mainTask;
    
    KLOG(LOG_LEVEL_INFO, "[Tasking] Multitasking initialized. Running mainTask.\n");
}

/**
 * @brief Creates a new task and initializes it with a stack and context.
 * 
 * @param task     Pointer to the Task structure to initialize.
 * @param main     Entry point function for the task.
 * @param flags    EFLAGS for the task.
 * @param pagedir  CR3 (page directory) that the task will use.
 */
void createTask(Task *task, void (*main), u32 flags, u32 *pagedir) {
    if ((!task) & (!main) & (!pagedir)) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] createTask received null pointer!\n");
        return;
    }

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Creating new task at %p for entry point %p\n", task, main);

    // Initialize registers with safe default values
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (u32) main;
    task->regs.cr3 = (u32) pagedir;

    // Allocate a new stack page and point ESP to its end
    task->regs.esp = (u32) alloc_task_stack_page() + 0x1000;

    task->next = 0;

    KLOG(LOG_LEVEL_INFO, "[Tasking] Task created: entry=0x%X, stack=0x%X\n", task->regs.eip, task->regs.esp);
}

/**
 * @brief Voluntarily yields CPU control to the next task.
 *
 * This function performs cooperative context switching between tasks.
 * It saves the state of the current task and restores the state of the next one.
 */
void yield(void) {
    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Yielding from task at EIP=0x%X\n", runningTask->regs.eip);

    // Save the current task and switch to the next
    Task *last = runningTask;
    runningTask = runningTask->next;

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Switching to task at EIP=0x%X\n", runningTask->regs.eip);
    
    // Perform the context switch
    switchTask(&last->regs, &runningTask->regs);
}
