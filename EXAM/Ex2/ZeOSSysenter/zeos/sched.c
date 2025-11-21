/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <types.h>
#include <hardware.h>
#include <segment.h>
#include <sched.h>
#include <mm.h>
#include <io.h>
#include <utils.h>
#include <p_stats.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

// Free task structs
struct list_head freequeue;
// Ready queue
struct list_head readyqueue;

void init_stats(struct stats *s)
{
	s->user_ticks = 0;
	s->system_ticks = 0;
	s->blocked_ticks = 0;
	s->ready_ticks = 0;
	s->elapsed_total_ticks = get_ticks();
	s->total_trans = 0;
	s->remaining_ticks = get_ticks();
}

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

/* ------------------- QUANTUM FUNCTIONS ------------------- */

#define DEFAULT_QUANTUM 10

int remaining_quantum=0; //queue ticks

/**
 * @brief Get the quantum value of a task
 * @param t Pointer to the task structure
 * @return The current quantum value of the task
 */
int get_quantum(struct task_struct *t)
{
  return t->total_quantum;
}

/**
 * @brief Set a new quantum value for a task
 * @param t Pointer to the task structure
 * @param new_quantum integer value for the new quantum
 */
void set_quantum(struct task_struct *t, int new_quantum)
{
  t->total_quantum=new_quantum;
}

/* Global variable to access easily to the idle task */
struct task_struct *idle_task=NULL;

/**
 * @brief Update scheduling data at each timer tick
 */
void update_sched_data_rr(void)
{
  remaining_quantum--; //queue ticks
}

/**
 * @brief Check if a task switch is needed
 * @return 1 if a task switch is needed, 0 otherwise
 */
int needs_sched_rr(void)
{
  /* If the current task has used up its quantum or if there are
   * other tasks in the ready queue, a task switch is needed */
  if ((remaining_quantum==0)&&(!list_empty(&readyqueue))) return 1;
  if (remaining_quantum==0) remaining_quantum=get_quantum(current());
  return 0;
}

/**
 * @brief Update the process state and move it to the specified queue
 * @param t Pointer to the task structure
 * @param dst_queue Pointer to the destination queue (NULL for running state)
 */
void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue)
{
  /* Remove from current queue if not running */
  if (t->state!=ST_RUN) list_del(&(t->list));
  
  /* Add to new queue */
  if (dst_queue!=NULL)
  {
    /* Add to the destination queue */
    list_add_tail(&(t->list), dst_queue);

    /* Update state based on queue type*/
    if (dst_queue!=&readyqueue) t->state=ST_BLOCKED;
    else
    {
      update_stats(&(t->p_stats.system_ticks), &(t->p_stats.elapsed_total_ticks));
      t->state=ST_READY;
    }
  }
  /* No destination queue means running state */
  else t->state=ST_RUN;
}

/**
 * @brief Select the next task to run and perform the task switch
 */
void sched_next_rr(void)
{
  struct list_head *e; //next
  struct task_struct *t; //next task

  /* Select the next task to run */
  if (!list_empty(&readyqueue)) {
	  /* Get the first element from the ready queue */
    e = list_first(&readyqueue);
    /* Remove it from the ready queue */
    list_del(e);
    /* Get the task struct from the list head */
    t=list_head_to_task_struct(e);
  }
  /* If the ready queue is empty, run the idle task */
  else {
    t=idle_task;
  }

  /* Perform the task switch */
  t->state=ST_RUN;
  remaining_quantum=get_quantum(t);

  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
  update_stats(&(t->p_stats.ready_ticks), &(t->p_stats.elapsed_total_ticks));
  t->p_stats.total_trans++;

  task_switch((union task_union*)t);
}

/**
 * @brief Main scheduling function called at each timer tick
 */
void schedule()
{
  update_sched_data_rr(); //update quantum counter
  
  // Check if we need to perform a context switch
  if (needs_sched_rr())
  {
    // Move current process to ready queue
    update_process_state_rr(current(), &readyqueue);
    // Select and switch to next process
    sched_next_rr();
  }
}

/* ------------------- INITIALIZATION FUNCTIONS ------------------- */
/**
 * @brief Initialize the idle task
 */
void init_idle (void)
{
  struct list_head *l = list_first(&freequeue);
  list_del(l);
  struct task_struct *c = list_head_to_task_struct(l);
  union task_union *uc = (union task_union*)c;

  c->PID=0;

  c->total_quantum=DEFAULT_QUANTUM;

  init_stats(&c->p_stats);

  allocate_DIR(c);

  uc->stack[KERNEL_STACK_SIZE-1]=(unsigned long)&cpu_idle; /* Return address */
  uc->stack[KERNEL_STACK_SIZE-2]=0; /* register ebp */

  c->register_esp=(int)&(uc->stack[KERNEL_STACK_SIZE-2]); /* top of the stack */

  idle_task=c;
}

/**
 * @brief Initialize task 1
 */
void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

/**
 * @brief Initialize task 1
 */
void init_task1(void)
{
  struct list_head *l = list_first(&freequeue);
  list_del(l);
  struct task_struct *c = list_head_to_task_struct(l);
  union task_union *uc = (union task_union*)c;

  c->PID=1;

  c->total_quantum=DEFAULT_QUANTUM;

  c->state=ST_RUN;

  remaining_quantum=c->total_quantum;

  init_stats(&c->p_stats);

  allocate_DIR(c);

  set_user_pages(c);

  tss.esp0=(DWord)&(uc->stack[KERNEL_STACK_SIZE]);
  setMSR(0x175, 0, (unsigned long)&(uc->stack[KERNEL_STACK_SIZE]));

  set_cr3(c->dir_pages_baseAddr);
}

/**
 * @brief Initialize the freequeue with all available task structs
 */
void init_freequeue()
{
  int i;

  INIT_LIST_HEAD(&freequeue);

  /* Insert all task structs in the freequeue */
  for (i=0; i<NR_TASKS; i++)
  {
    task[i].task.PID=-1;
    list_add_tail(&(task[i].task.list), &freequeue);
  }
}

/**
 * @brief Initialize the scheduler
 */
void init_sched()
{
  init_freequeue();
  INIT_LIST_HEAD(&readyqueue);
}

/* ------------------- TASK SWITCHING FUNCTIONS ------------------- */
/**
 * @brief Get the current running task
 * @return Pointer to the current task structure
 */
struct task_struct* current()
{
  int ret_value;
  //falta algo???
  return (struct task_struct*)( ((unsigned int)&ret_value) & 0xfffff000);
}

/**
 * @brief Convert a list head pointer to a task structure pointer
 * @param l Pointer to the list head
 * @return Pointer to the corresponding task structure
 */
struct task_struct* list_head_to_task_struct(struct list_head *l)
{
  return (struct task_struct*)((int)l&0xfffff000);
}

/* Do the magic of a task switch */
void task_switch(union task_union *new)
{
  page_table_entry *new_DIR = get_DIR(&new->task);

  /* Update TSS and MSR to make it point to the new stack */
  tss.esp0=(int)&(new->stack[KERNEL_STACK_SIZE]);
  setMSR(0x175, 0, (unsigned long)&(new->stack[KERNEL_STACK_SIZE]));

  /* TLB flush. New address space */
  set_cr3(new_DIR);

  switch_stack(&current()->register_esp, new->task.register_esp);
}


/* Force a task switch assuming that the scheduler does not work with priorities */
void force_task_switch()
{
  update_process_state_rr(current(), &readyqueue);

  sched_next_rr();
}
