// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Quantum constant in MLFQ 3-level queue
#define MLFQ_QUANTUM_0 1
#define MFLQ_QUANTUM_1 2
#define MLFQ_QUANTUM_2 4
// Allotment constant in MLFQ 3-level queue
#define MLFQ_ALLOTMENT_0 5
#define MLFQ_ALLOTMENT_1 10
// Boosting constant in MLFQ 3-level queue
#define BOOSTING_TICKS 100


// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  // Project 1
  // Elements for Scheduler
  int mode;                    // MLFQ: 1, Stride: 0
  int level;                   // MLFQ: 0~2, Stride: -1
  int priority;                // Check the priority of process in queue
  uint ticknum;                // Check ticknum in MLFQ scheduling
  int portion;                 // CPU share portion, MLFQ: 0
  double tickets;              // The number of tickets for this process
  double stride;               // Stride of this process
  double pass;                 // Counter for stride scheduling
  int isYield;                 // Flag for checking yield() call
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
