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

#include <stdint.h>
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

static Task shellTask;

/// External: allocates a page for the stack of a new task (defined elsewhere)
extern void* alloc_task_stack_page(void);

extern void shell_task(void);

static uint32_t next_taskId = 1;

/**
 * @brief Initializes the cooperative multitasking subsystem.
 *
 * This function sets up the main task, creates a secondary task (otherMain),
 * and links both in a circular list. It then calls yield() to begin
 * context switching to the other task.
 */
void initTasking(void) {
    // Capture CR3 (current page directory) and flags (EFLAGS)
    __asm__ __volatile__ ("movl %%cr3, %%eax; movl %%eax, %0;\n" : "=m"(mainTask.regs.cr3) :: "%eax");
    __asm__ __volatile__ ("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;\n" : "=m"(mainTask.regs.eflags) :: "%eax");

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] mainTask CR3: 0x%X, EFLAGS: 0x%X\n", mainTask.regs.cr3, mainTask.regs.eflags);

    // Create secondary task
    createTask(&shellTask, shell_task, mainTask.regs.eflags, (uint32_t*)mainTask.regs.cr3);

    // Link the tasks in a circular list
    mainTask.next = &shellTask;
    shellTask.next = &mainTask;

    runningTask = &mainTask;
    
    KLOG(LOG_LEVEL_INFO, "[Tasking] Multitasking initialized. Running mainTask\n");
    KLOG(LOG_LEVEL_DEBUG, "Running task id=%u\n", runningTask->taskId);
}

/**
 * @brief Creates a new task and initializes it with a stack and context.
 * 
 * @param task     Pointer to the Task structure to initialize.
 * @param main     Entry point function for the task.
 * @param flags    EFLAGS for the task.
 * @param pagedir  CR3 (page directory) that the task will use.
 */
void createTask(Task *task, void (*main), uint32_t flags, uint32_t *pagedir) {
    if ((!task) & (!main) & (!pagedir)) {
        KLOG(LOG_LEVEL_ERROR, "[Tasking] createTask received null pointer!\n");
        KLOG(LOG_LEVEL_DEBUG, "Running task id=%d\n", task->taskId);
        return;
    }

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Creating new task at %pfor entry point %p\n", task, main);

    task->taskId = next_taskId++;
    // Initialize registers with safe default values
    task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = flags;
    task->regs.eip = (uint32_t) main;
    task->regs.cr3 = (uint32_t) pagedir;

    // Allocate a new stack page and point ESP to its end
    task->regs.esp = (uint32_t) alloc_task_stack_page() + 0x1000;
    task->regs.ebp = task->regs.esp;    // <— initialize EBP to the top of stack

    task->next = 0;

    KLOG(LOG_LEVEL_INFO, "[Tasking] Task created: id=%u entry=0x%X, stack=0x%X\n", task->taskId, task->regs.eip, task->regs.esp);
}

/**
 * @brief Voluntarily yields CPU control to the next task.
 *
 * This function performs cooperative context switching between tasks.
 * It saves the state of the current task and restores the state of the next one.
 */
void yield(void) {
    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Yielding from task at EIP=0x%X TASK ID=%u\n", runningTask->regs.eip, runningTask->taskId);

    // Save the current task and switch to the next
    Task *last = runningTask;
    runningTask = runningTask->next;

    KLOG(LOG_LEVEL_DEBUG, "[Tasking] Switching to task at EIP=0x%X TASK ID=%u\n", runningTask->regs.eip, runningTask->taskId);
    KLOG(LOG_LEVEL_DEBUG, "Task %d context: EIP=0x%x ESP=0x%x EBP=0x%x\n", runningTask->taskId, runningTask->regs.eip, runningTask->regs.esp, runningTask->regs.ebp);
    
    // Perform the context switch
    switchTask(&last->regs, &runningTask->regs);
}
