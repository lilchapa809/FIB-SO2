/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

//Defined in assembly code
void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

/* Update process stats when switching from user to system mode */
void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

/* Update process stats when switching from system to user mode */
void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

/**
 * @brief La función writeTo permite enviar size bytes desde src al proceso lector pid.
 *  A continuación, el proceso actual debe quedarse bloqueado en la
 * lista de procesos pendientes del proceso lector a que el readFrom lo desbloquee. Al
 * desbloquearse, el proceso debe mapear la página física del proceso lector en la
 * dirección lógica 0x3FF000, copiar la región de memoria desde src, desmapear la
 * página física, desbloquear al lector y devolver el número de bytes escritos. Para
 * simplificar, sólo permitimos escrituras de hasta 4096 bytes.
 * 
 * @param pid Identificador del proceso lector
 * @param src Puntero a la región de memoria origen
 * @param size Número de bytes a escribir
 * @return Número de bytes escritos o código de error
 * @note Si el proceso lector está bloqueado y 
 * no tiene ningún escritor activo, debemos desbloquearlo.
 */
int sys_writeTo(int pid, char* src, int size){
  //"Puedes ignorar la gestión de errores de estas llamadas"
    //asumimos que 0 <= size <=4096 
    //asumimos que el pid es válido (existe el proceso lector)
    //asumimos que src es una dirección válida

  //inicializaciones
  struct task_struct *reader_task;
  struct task_struct *current_task = current();
  page_table_entry *current_PT = get_PT(current_task);
  int written_bytes = 0;

  reader_task = get_task_by_pid(pid);

  //Comprobar si hay un escritor activo
  if (reader_task->comm.active_writer == NULL) {
    //No hay escritor activo
    //Asignar el escritor activo
    reader_task->comm.active_writer = current_task;
    //Mapear la página física del lector en la dirección lógica 0x3FF000
    mapPhysicalPageFrom(reader_task, COMM_PAGE_ADDR);
    //Copiar los datos desde src a la página de información
    copy_from_user(src, (char*)COMM_PAGE_ADDR, size);
    //Desmapear la página física
    unmapPage(reader_task, COMM_PAGE_ADDR);
    //Actualizar el número de bytes escritos
    reader_task->comm.bytes_written = size;
    //Desbloquear al lector
    if (reader_task->state == ST_BLOCKED) {
      update_process_state_rr(reader_task, &readyqueue);
    }
    //Bloquear al escritor actual
    update_process_state_rr(current_task, &reader_task->comm.blocked_writers);
    //Forzar un cambio de tarea
    force_task_switch();
    //Al desbloquearse, devolver el número de bytes escritos
    return size;
  } else {
    //Hay un escritor activo
    //Bloquear al escritor actual
    update_process_state_rr(current_task, &reader_task->comm.blocked_writers);
    //Forzar un cambio de tarea
    force_task_switch();
    //Al desbloquearse, devolver 0 (no se ha escrito nada)
    return 0;
    }
}

/**
 * @brief La función readFrom permite leer la información enviada por otros procesos.
 * Caso que haya alguien, debe marcarlo como el actual escritor activo, desbloquearlo y
 * bloquearse a que este escritor escriba los datos. Al desbloquearse, debe mapear la
 * página física en la dirección 0x3FD000, copiar tantos bytes como se hayan escrito a
 * partir de la dirección dst, desmapear la página, marcar que ya no hay escritor activo,
 * copiar el pid del escritor en el parámetro pid y devolver el número de bytes leídos. 
 * @param pid Puntero donde se devolverá el identificador del proceso escritor
 * @param dst Puntero a la región de memoria destino
 * @return Número de bytes leídos o código de error
 * @note Para ello, si no hay nadie bloqueado para escribir, el proceso actual debe bloquearse.
 * */
int sys_readFrom(int* pid, char *dst){
   //"Puedes ignorar la gestión de errores de estas llamadas"
    //asumimos que el pid es válido (existe el proceso escritor)
    //asumimos que dst es válido

  //inicializaciones
  struct task_struct *current_task = current();
  page_table_entry *current_PT = get_PT(current_task);
  struct task_struct *writer_task;

  //Comprobar si hay escritores bloqueados
  if (list_empty(&current_task->comm.blocked_writers)) {
    //No hay escritores bloqueados
    //Bloquear al proceso actual
    update_process_state_rr(current_task, &current_task->comm.blocked_writers);
    //Forzar un cambio de tarea
    force_task_switch();

    //Al desbloquearse, obtener el escritor activo
    writer_task = current_task->comm.active_writer;
    //Mapear la página física del escritor en la dirección lógica 0x3FD000
    mapPhysicalPageFrom(writer_task, READ_COMM_PAGE_ADDR);
    //Copiar los datos desde la página de información a dst
    copy_to_user((char*)READ_COMM_PAGE_ADDR, dst, writer_task->comm.bytes);
    //Desmapear la página física
    unmapPage(writer_task, READ_COMM_PAGE_ADDR);
    //Obtener el pid del escritor
    *pid = writer_task->PID;
    //Marcar que no hay escritor activo
    current_task->comm.active_writer = NULL;
    //Desbloquear al escritor
    update_process_state_rr(writer_task, &readyqueue);
    //Devolver el número de bytes leídos
    return writer_task->comm.bytes;
  } else {
    //Hay escritores bloqueados
    //Obtener el primer escritor bloqueado
    struct list_head *e = list_first(&current_task->comm.blocked_writers);
    writer_task = list_head_to_task_struct(e);
    //Eliminar de la lista de bloqueados
    list_del(e);
    //Marcar como escritor activo
    current_task->comm.active_writer = writer_task;
    //Mapear la página física del escritor en la dirección lógica 0x3FD000
    mapPhysicalPageFrom(writer_task, READ_COMM_PAGE_ADDR);
    //Copiar los datos desde la página de información a dst
    copy_to_user((char*)READ_COMM_PAGE_ADDR, dst, writer_task->comm.bytes);
    //Desmapear la página física
    unmapPage(writer_task, READ_COMM_PAGE_ADDR);
    //Obtener el pid del escritor
    *pid = writer_task->PID;
    //Marcar que no hay escritor activo
    current_task->comm.active_writer = NULL;
    //Desbloquear al escritor
    update_process_state_rr(writer_task, &readyqueue);
    //Devolver el número de bytes leídos
    return writer_task->comm.bytes;
  }
}