/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <hardware.h>

char initial_stack[KERNEL_STACK_SIZE]; /* pila del main abans del multitasking */

struct list_head readyqueue; /* Ready queue */
struct task_struct *init_task; /* Pointer to the init task */
struct task_struct *idle_task; /* Pointer to the idle task */

void cpu_idle(void)
{
	__sti();
	while(1)
	{
	;
	}
}

void init_idle (void)
{
	/* 1) Allocate frames for the page tables */
	int dir_frame = alloc_frame();
	int pcb_frame = alloc_frame();

	/* 2) Create page tables */
	page_table_entry *dir = (page_table_entry *)(dir_frame << 12);
	union task_union *idle_union = (union task_union *)(pcb_frame << 12);
	struct task_struct *task = &idle_union->task;
	page_table_entry *init_spt = get_PT(init_task);

	/* 3) Idle only needs a directory and reuses init system page table */
	clear_page_table(dir);
	dir[0] = init_task->dir_pages_baseAddr[0];

	/* 4) Keep idle structures mapped in the system page table */
	set_ss_pag(init_spt, dir_frame, dir_frame, 0);
	set_ss_pag(init_spt, pcb_frame, pcb_frame, 0);

	/* 5) Initialize task metadata */
	task->PID = 0;
	task->dir_pages_baseAddr = dir;
	task->state = ST_READY;
	INIT_LIST_HEAD(&task->list);

	/* 6) Prepare first context restore to execute cpu_idle() */
	idle_union->stack[KERNEL_STACK_SIZE - 2] = 0;
	idle_union->stack[KERNEL_STACK_SIZE - 1] = (DWord)&cpu_idle;
	task->kernel_esp = (DWord)&idle_union->stack[KERNEL_STACK_SIZE - 2];

	/* 8) Keep global pointer to idle task */
	idle_task = task;
}

void init_task1(void)
{
	/* 1) Allocate frames for the page tables */
	int dir_frame = alloc_frame();
	int spt_frame = alloc_frame();
	int upt_frame = alloc_frame();
	int pcb_frame = alloc_frame();

	/* 2) Create page tables */
	page_table_entry *dir = (page_table_entry *)(dir_frame << 12);
	page_table_entry *spt = (page_table_entry *)(spt_frame << 12);
	page_table_entry *upt = (page_table_entry *)(upt_frame << 12);
	union task_union *init_union = (union task_union *)(pcb_frame << 12);
	struct task_struct *task = &init_union->task;

	/* 3) Build address space for init process */
	clear_page_table(dir);
	clear_page_table(spt);
	clear_page_table(upt);
	set_kernel_pages(spt);
	set_user_pages(upt);

	/* 4) Identity-map system structures to keep direct access */
	set_ss_pag(spt, dir_frame, dir_frame, 0);
	set_ss_pag(spt, spt_frame, spt_frame, 0);
	set_ss_pag(spt, upt_frame, upt_frame, 0);

	/* 5) Map PCB in system page table */
	set_ss_pag(spt, pcb_frame, pcb_frame, 0);

	/* 6) Directory[0] -> system page table */
	dir[0].entry = 0;
	dir[0].bits.pbase_addr = spt_frame;
	dir[0].bits.present = 1;
	dir[0].bits.rw = 1;

	/* 7) Directory[1] -> user page table (user permissions) */
	dir[1].entry = 0;
	dir[1].bits.pbase_addr = upt_frame;
	dir[1].bits.present = 1;
	dir[1].bits.rw = 1;
	dir[1].bits.user = 1;

	/* 8) PID of init process is always 1 */
	task->PID = 1;

	/* 9) Store page directory */
	task->dir_pages_baseAddr = dir;
	task->state = ST_RUN;
	INIT_LIST_HEAD(&task->list);

	/* 10) Update TSS.ESP0 and SYSENTER_ESP_MSR (0x175) */
	tss.esp0 = KERNEL_ESP(init_union);
	writeMSR(0x175, (DWord)tss.esp0);

	/* 11) Activate init address space */
	set_cr3(dir);

	/* 12) Keep global pointer to init process */
	init_task = task;
}


void init_sched(void)
{
	INIT_LIST_HEAD(&readyqueue);
}

void inner_task_switch(union task_union *new)
{
	/* 1) Update kernel stack used when entering kernel mode */
	tss.esp0 = KERNEL_ESP(new);
	writeMSR(0x175, (DWord)tss.esp0);

	/* 2) Change user address space (flushes TLB) */
	set_cr3(get_DIR(&new->task));

	/* 3-6) Save current EBP, load new stack, restore EBP and RET */
	switch_stack(&current()->kernel_esp, new->task.kernel_esp);
}

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
	return list_entry(l, struct task_struct, list);
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

