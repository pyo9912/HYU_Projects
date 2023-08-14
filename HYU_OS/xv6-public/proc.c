#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

double MLFQ_tickets = 100;      // Portion of MLFQ's CPU share
double stride_tickets = 0;      // Portion of Stride's CPU share

double MLFQ_stride = 0;              // Check the stride of scheduler
double MLFQ_pass = 0;                // Check the pass of scheduler
double stride_pass = 0;

int boosting_ticks = 0;         // Check ticks for boosting in MLFQ
int highestPriority = 0;        // Update the highestPriority;

int nextpid = 1;                // use for process management
int nexttid = 1;                // use for thread management
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    p->pass = 0;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  // Project1
  // Init for MLFQ + stride scheduler
  p->mode = 1;
  p->level = 0;
  p->priority = 0;
  p->ticknum = 0;
  p->portion = 0;
  p->stride = 0;
  p->pass = 0;
  p->isYield = 0;
  // Project2
  p->tid = 0;
  p->master = 0;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc * p;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == curproc->pid) {
      p->sz = sz;
    }
  }
  release(&ptable.lock);
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Set pgdir
  if (curproc->tid == 0) {
    np->pgdir = copyuvm(curproc->pgdir, curproc->sz);
  }
  else {
    np->pgdir = copyuvm(curproc->pgdir, curproc->master->sz);
  }

  // Copy process state from proc.
  if(np->pgdir == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == curproc->pid) {
      for(fd = 0; fd < NOFILE; fd++){
        if(curproc->ofile[fd]){
          fileclose(curproc->ofile[fd]);
          curproc->ofile[fd] = 0;
        }
      }

      begin_op();
      iput(curproc->cwd);
      end_op();
      curproc->cwd = 0;
    }
  }
  

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Adjust the tickets and stride
  MLFQ_tickets += curproc->portion;
  stride_tickets -= curproc->portion;
  MLFQ_stride = (double)(100/MLFQ_tickets);

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
// Project1
// Modify scheduler into MLFQ + Stride Scheduler 

// Project 1
// MLFQ Scheduling
// The scheduler choose a next ready process form MLFQ.
// Check the level in proc structure instead of making queue
// RR policy quantum -> level 0: 1 tick / level 1: 2 ticks / level 2: 4 ticks
// Time allotment -> level 0: 5 ticks / level 1: 10 ticks
// Boosting to prevent starvation
void
MLFQ_scheduler(void)
{
  struct proc* p;
  struct cpu* c = mycpu();

  // Boosting
  if (boosting_ticks >= BOOSTING_TICKS) {
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->state != RUNNABLE)
        continue;

      p->level = 0;
      p->ticknum = 0;
    }
    boosting_ticks = 0;
    highestPriority = 0;
  }

  int cur_level = 2;
  int minPriority = 99999999;
  struct proc* ret = 0;

  // Get process by RR policy
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->state != RUNNABLE || p->mode != 1)
      continue;

    if (p->level < cur_level) {
      cur_level = p->level;
      minPriority = p->priority;
      ret = p;
    }
    else if (p->level == cur_level && p->priority < minPriority) {
      minPriority = p->priority;
      ret = p;
    }
  }

  if((p = ret)) {
    c->proc = p;
    switchuvm(p);
    p->state = RUNNING;
    p->isYield = 0;

    swtch(&(c->scheduler), p->context);
    switchkvm();

    p->ticknum++;
    boosting_ticks++;

    c->proc = 0;

    if (p->level == 0) {
      if (p->ticknum >= MLFQ_ALLOTMENT_0) {
        p->level++;
        p->ticknum = 0;
      }
      else if (p->ticknum >= MLFQ_QUANTUM_0) {
        p->priority = ++highestPriority;
      }
    }
    else if (p->level == 1) {
      if (p->ticknum >= MLFQ_ALLOTMENT_1) {
        p->level++;
        p->ticknum = 0;
      }
      else if (p->ticknum >= MFLQ_QUANTUM_1) {
        p->priority = ++highestPriority;
      }
    }
    else if (p->level == 2) {
      if (p->ticknum >= MLFQ_QUANTUM_2) {
        p->priority = ++highestPriority;
      }
    }
  }
  
  MLFQ_pass += MLFQ_stride;
}


// Project1
// Stride Scheduling
// The scheduler choose the process which has the lowest pass value
void
stride_scheduler(void)
{
  struct proc* p;
  struct cpu* c = mycpu();

   c->proc = 0;

  int i;
  double min_pass_val = 99999999;
  int min_pass_idx = 0;

  // Loop to get the index of process which has lowest pass value
  for (i=0; i<NPROC; i++) {
      p = ptable.proc + i;
      if (p->state != RUNNABLE)
        continue;
      if (p->mode != 0)
        continue;
      if (p->pass < min_pass_val) {
        min_pass_val = p->pass;
        min_pass_idx = i;
      }
  }

  // Get the process which has lowest pass value
  p = ptable.proc + min_pass_idx;

  // Switch to chosen process.  It is the process's job
  // to release ptable.lock and then reacquire it
  // before jumping back to us.
  c->proc = p;
  switchuvm(p);
  
  p->pass += p->stride;
  stride_pass += p->stride;
  p->state = RUNNING;

  swtch(&(c->scheduler), p->context);
  switchkvm();

  // Process is done running for now.
  // It should have changed its p->state before coming back.
  c->proc = 0;
}

void
scheduler(void)
{
  for(;;){
    sti();

    acquire(&ptable.lock);
    if (MLFQ_tickets == 100) {
      MLFQ_scheduler();
    }
    else if(MLFQ_tickets != 100 && MLFQ_pass <= stride_pass) {
      MLFQ_scheduler();
    }
    else {
      stride_scheduler();
    }
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

void
self_yield(void)
{
  myproc()->isYield = 1;
  yield();
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid && p->tid == 0){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// Lab Practice
int
getppid(void)
{
  return myproc()->parent->pid;
}

// Project 1
// Get the level of queue in MLFQ
int
getlev(void)
{
  struct proc* p = myproc();
  
  if (p == 0)
    return -1;

  return p->level;
}

// Project 1
// Inquires to obtain CPU share percent
// Set the amount of CPU share for stride scheduling
int
set_cpu_share(int n)
{
  struct proc* p;
  struct proc* curproc = myproc();
  
  // Make sure that MLFQ occupy at least 20% of CPU share
  // Limit stride share occupy at most 80% of CPU share
  if (MLFQ_tickets - n < 20)
    return -1;

  // Update values in process
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == curproc->pid) {
      p->mode = 0;        // mode: init as stride scheduling
      p->level = -1;      // level: init as stride scheduling
      p->priority = 0;    
      p->ticknum = 0;
      p->portion = (double) n;
      p->stride = (double) 100 / n;
      p->pass = 0;
      p->isYield = 0;
    }
  }

  // Update CPU tickets
  MLFQ_tickets -= n;
  stride_tickets += n;

  MLFQ_stride = (double)(100/MLFQ_tickets);
  MLFQ_pass = 0;
  stride_pass = 0;

  release(&ptable.lock);

  return 0;
}

// Project2
// Provide a method to create threads within the process.
int
thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg)
{
  int i;
  struct proc* p;
  struct proc* np;
  struct proc* curproc = myproc();
  pde_t* pgdir;
  uint sz, sp, ustack[2], base;
  // Allocate process
  if ((np = allocproc()) == 0) {
    return -1;
  }

  --nextpid;
  // Set thread elements
  np->pgdir = curproc->pgdir;
  np->pid = curproc->pid;
  np->parent = curproc->parent;
  np->tid = nexttid++;
  *np->tf = *curproc->tf;
  if (curproc->tid == 0) {
    // Case: curproc is a master thread
    np->master = curproc;
  }
  else {
    // Case: curproc is a worker thread
    np->master = curproc->master;
  }
  np->master->num_thread++;
  pgdir = np->master->pgdir;
  // Copy master thread's data as fork() does
  for (i = 0; i < NOFILE; i++) {
    if (curproc->ofile[i]) {
      np->ofile[i] = filedup(curproc->ofile[i]);
    }
  }
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));
  // Return thread id
  *thread = np->tid;

  // Allocate user stack
  base = np->master->sz;
  np->master->sz += 2*PGSIZE;

  if ((sz = allocuvm(pgdir, base, base + 2*PGSIZE)) == 0) {
    np->state = UNUSED;
    return -1;
  }

  sp = sz;

  ustack[0] = 0xFFFFFFFF;
  ustack[1] = (uint)arg;
  sp -= 8;

  if (copyout(np->pgdir, sp, ustack, 8) < 0) {
    return -1;
  }
  np->sz = sz;
  // Set start_routine
  np->tf->eip = (uint)start_routine;
  // Set stack pointer
  np->tf->esp = sp;

  // Adjust scheduler elements
  acquire(&ptable.lock);
  if (curproc->portion != 0) {
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->pid == curproc->pid) {
        // Init element as stride (default)
        p->mode = 0;
        p->level = -1;
        p->portion = curproc->portion;
        p->tickets = curproc->tickets / (double)(curproc->num_thread + 1);
        p->stride = (double)100 / p->tickets;
      }
    }
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      p->pass = 0;
    }
    // Init pass value
    MLFQ_pass = 0;
    stride_pass = 0;
  }
  np->state = RUNNABLE;
  release(&ptable.lock);

  // Return 0 if success
  return 0;
}

// Project2
// Terminate the thread.
// Return the result of a thread
// Struct is similar to exit function
void
thread_exit(void *retval)
{
  int i;
  struct proc* p;
  struct proc* curproc = myproc();

  // Close all open files
  for (i = 0; i < NOFILE; i++) {
    if (curproc->ofile[i]) {
      fileclose(curproc->ofile[i]);
      curproc->ofile[i] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);
  
  // Check master thread and wake it up
  wakeup1(curproc->master);

  // Update scheduling elements
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (curproc->tickets != 0) {
      for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == curproc->pid && p->tid == curproc->tid) {
          p->tickets = curproc->tickets / (double)(curproc->master->num_thread+1);
          p->stride = (double)(100/p->tickets);
        }
      }
    }
  }

  // Jump to scheduler, never to return
  curproc->ret_val = retval;
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");

}

// Project2
// Wait for the thread specified by the argument to terminate
// If the tread has already terminated, it returns immediately
// In join, clean up resources allocated to the thread
// e.g. page table, allocated memories, stacks
int
thread_join(thread_t thread, void **retval)
{
  struct proc* p;
  struct proc* curproc = myproc();

  // Only for master thread
  if (curproc->master != 0) {
    return -1;
  }

  acquire(&ptable.lock);

  for (;;) {
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->tid != thread) {
        continue;
      }

      if (p->master != curproc) {
        release(&ptable.lock);
        return -1;
      }

      if (p->state == ZOMBIE) {
        *retval = p->ret_val;
        
        // Clean up thread
        kfree(p->kstack);
        p->kstack = 0;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->master = 0;
        p->state = UNUSED;

        release(&ptable.lock);
        return 0;
      }
    }

    if (curproc->killed) {
      release(&ptable.lock);
      return -1;
    }

    // Wait for worker thread to exit
    sleep(curproc, &ptable.lock);
  }
}