#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getppid(void)
{
  return getppid();
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Syscall in Project 1
int
sys_yield(void)
{
  self_yield();
  return 0;
}

// Syscall in Project 1
int
sys_getlev(void)
{
  return getlev();
}

// Syscall in Project 1
// Wrapper function for set_cpu_share(int)
int
sys_set_cpu_share(void)
{
  int n;

  if (argint(0, &n) < 0)
    return -1;

  return set_cpu_share(n);
}

// Syscall in Project 2
int
sys_thread_create(void)
{
  int n;
  thread_t* thread;
  void* (*start_routine)(void*);
  void* arg;

  if (argint(0, &n) < 0)
    return -1;
  
  thread = (thread_t*)n;

  if (argint(1, &n) < 0)
    return -1;
  
  start_routine = (void*)n;

  if (argint(2, &n) < 0)
    return -1;

  arg = (void*)n;

  return thread_create(thread, start_routine, arg);
}

// Syscall in Project 2
int
sys_thread_exit(void)
{
  int n;
  void* ret_val;

  if (argint(0, &n) < 0)
    return -1;

  ret_val = (void*)n;

  thread_exit(ret_val);

  return 0;
}

// Syscall in Project 2
int
sys_thread_join(void)
{
  int n;
  thread_t thread;
  void** ret_val;

  if (argint(0, &n) < 0)
    return -1;

  thread = (thread_t)n;

  if (argint(1, &n) < 0)
    return -1;

  ret_val = (void**)n;

  return thread_join(thread, ret_val);
}