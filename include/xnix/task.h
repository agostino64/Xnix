#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "isr.h"
#include "paging.h" // Required for page_directory_t

/**
 * @file task.h
 * @brief Public interface for the preemptive multitasking system.
 *
 * Defines the core structures and functions for creating and managing tasks,
 * including the scheduler and context switching mechanisms.
 */

/**
 * @struct Task
 * @brief Represents a single task (or process) in the system.
 *
 * Each task has a unique ID, a saved register state for context switching,
 * a pointer to its address space (page directory), and a link to the next
 * task in a circular list, which enables round-robin scheduling.
 */
typedef struct Task {
    int id;               ///< Unique identifier for the task.
    registers_t regs;     ///< Saved CPU state (registers, flags, instruction pointer).
    struct Task *next;    ///< Pointer to the next task in the circular list.
    page_directory_t *page_directory; ///< The address space for this task.
} Task;

/**
 * @brief Initializes the preemptive multitasking system.
 *
 * This function must be called once at kernel startup. It sets up the initial
 * kernel task, creates the first user-space task (e.g., a shell), and
 * prepares the scheduler for the first context switch.
 */
void init_tasking(void);

/**
 * @brief Creates a new task and adds it to the scheduler's list.
 *
 * @param parent The task after which the new task will be inserted in the list.
 * @param new_task A pointer to an uninitialized Task structure to be configured.
 * @param entry_point The function pointer where the new task will begin execution.
 * @param task_page_dir The page directory (address space) for the new task.
 * @return 0 on success, -1 on failure (e.g., null pointers, memory allocation error).
 */
int create_task(Task *parent, Task *new_task, void (*entry_point)(void), page_directory_t *task_page_dir);

/**
 * @brief Voluntarily yields CPU control to the next task.
 *
 * Triggers a software interrupt (int 0x80) to invoke the scheduler. This provides a
 * mechanism for cooperative multitasking, allowing a task to give up its
 * time slice before it expires.
 */
void yield(void);

/**
 * @brief The core scheduler function for task switching.
 *
 * This function is invoked by an interrupt (either the PIT timer for
 * preemption or a software interrupt for yielding). It saves the current
 * task's state and switches to the next task in the round-robin list.
 *
 * @param regs Pointer to the register frame saved by the interrupt stub.
 */
void schedule(registers_t *regs);

#endif /* __TASK_H__ */
