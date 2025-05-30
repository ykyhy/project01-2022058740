#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
extern int scheduler_mode;
extern int tick_count;
extern struct proc proc[NPROC];
extern struct spinlock ptable_lock;

#define FCFS_MODE 0
#define MLFQ_MODE 1


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_getppid(void)
{
	return myproc()->parent->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_yield(void)
{
    yield();
    return 0;
}

uint64
sys_getlev(void)
{
    struct proc *p = myproc();
    if (scheduler_mode == FCFS_MODE)
        return 99;
    return p->level;
}

uint64
sys_setpriority(void)
{
    int pid, priority;
    struct proc *p;

    argint(0, &pid);     
    argint(1, &priority);

    if (priority < 0 || priority > 3)
        return -2; 

    acquire(&ptable_lock);
    for (p = proc; p < &proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->priority = priority;
            release(&ptable_lock);
            return 0;  
        }
    }
    release(&ptable_lock);

    return -1; 
}

uint64
sys_mlfqmode(void)
{
    if (scheduler_mode == MLFQ_MODE) {
        return -1;
    }

    acquire(&ptable_lock);

	printf("CPU %d: Switching to MLFQ mode\n", cpuid());
    
	scheduler_mode = MLFQ_MODE;

    for (struct proc *p = proc; p < &proc[NPROC]; p++) {
        if (p->state != UNUSED) {
            p->priority = 3;
            p->level = 0;
            p->time_quantum = 0;
        }
    }
    tick_count = 0;
    release(&ptable_lock);

    return 0;
}

uint64
sys_fcfsmode(void)
{
    if (scheduler_mode == FCFS_MODE) {
        return -1;
    }

    acquire(&ptable_lock);
    scheduler_mode = FCFS_MODE;

    for (struct proc *p = proc; p < &proc[NPROC]; p++) {
        if (p->state != UNUSED) {
            p->priority = -1;
            p->level = -1;
            p->time_quantum = -1;
        }
    }
    tick_count = 0;
    release(&ptable_lock);
    return 0;
}

