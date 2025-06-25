#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "common.h"
#include "isr.h"

extern void initTasking(void);

#define ADDR_PAGE_TASK 0xE0000000  // High virtual address for task stacks (you can adjust)

typedef struct cpu_state {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} CPUState;

typedef struct Task {
    CPUState regs;
    struct Task *next;
    uint32_t taskId;
} Task;

void initTasking(void);
void createTask(Task*, void(*), uint32_t, uint32_t*);

void yield(void); // Switch task frontend
void switchTask(CPUState *old, CPUState *new); // The function which actually switches

void switchTask(CPUState *from, CPUState *to);

#endif /* __TASK_H__ */
