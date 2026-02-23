/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>


#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct pipe {
    int frame; //physical frame to store data
    int numbytes; //number of bytes in page
    struct task_struct* readingFrom; //active writer
    struct list_head writers; //list of pending writers
};

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;	/* Task struct enqueuing */
  int register_esp;		/* position in the stack */
  enum state_t state;		/* State of the process */
  int total_quantum;		/* Total quantum of the process */
  struct stats p_stats;		/* Process stats */

  struct pipe pipeinfo; //Examen

};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union protected_tasks[NR_TASKS+2];
extern union task_union *task; /* Vector de tasques */
extern struct task_struct *idle_task;


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

extern struct list_head freequeue;
extern struct list_head readyqueue;

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void schedule(void);

struct task_struct * current();

void task_switch(union task_union*t);
void switch_stack(int * save_sp, int new_sp);

void sched_next_rr(void);

void force_task_switch(void);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void init_stats(struct stats *s);

void initializeData(struct task_struct*p);

int mapPhysicalPageFrom(struct task_struct* p, int address);
int unmapPage(struct task_struct*p, int address);

int mapPhysicalPageFrom(struct task_struct* p, int address) {
    int frame = p->pipeinfo.frame;
    page_table_entry *myPT = get_PT(current());
    set_ss_page(myPT, address>>12, frame);
}

int unmapPage(struct task_struct*p, int address) {
    page_table_entry *myPT = get_PT(current());
    del_ss_page(myPT, address>>12);
    set_cr3(get_DIR(p));
}


#endif  /* __SCHED_H__ */
