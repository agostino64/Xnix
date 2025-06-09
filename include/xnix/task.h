#ifndef __TASK_H__
#define __TASK_H__

#include "common.h"
#include "isr.h"

extern void initTasking(void);

#define ADDR_PAGE_TASK 0xE0000000  // High virtual address for task stacks (you can adjust)

typedef struct cpu_state {
    u32 eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} CPUState;

typedef struct Task {
    CPUState regs;
    struct Task *next;
    u32 taskId;
} Task;

void initTasking(void);
void createTask(Task*, void(*), u32, u32*);

void yield(void); // Switch task frontend
void switchTask(CPUState *old, CPUState *new); // The function which actually switches

void switchTask(CPUState *from, CPUState *to);

#endif /* __TASK_H__ */
