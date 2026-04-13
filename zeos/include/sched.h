/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr; /* Pointer to the page directory of the process */
  //struct task_struct *parent; /* Pointer to the parent process */
  //struct list_head children; /* To enqueue the children in the children list */
  //struct list_head sibling; /* To enqueue the siblings in the siblings list */
  struct list_head list; /* To enqueue the process in the ready queue */
  enum state_t state; /* Process state */
  unsigned long kernel_esp; /* ESP saved during a context switch */
  //int total_quantum; /* Total quantum of the process */
  //int remaining_quantum; /* Remaining quantum of the process */
  //int pending_unblocks; /* Number of pending unblocks */

};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern struct list_head readyqueue; /* Ready queue */
extern struct task_struct *init_task; /* Pointer to the init task */
extern struct task_struct *idle_task; /* Pointer to the idle task */

/* Converts a list head to a task struct */
struct task_struct *list_head_to_task_struct(struct list_head *l);

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

extern char initial_stack[KERNEL_STACK_SIZE];
#define INITIAL_ESP             (DWord) &initial_stack[KERNEL_STACK_SIZE]

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union *new);
void inner_task_switch(union task_union *new);
void switch_stack(unsigned long *old_ebp, unsigned long new_ebp);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

#endif  /* __SCHED_H__ */
