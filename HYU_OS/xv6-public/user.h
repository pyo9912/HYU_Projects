struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int myfunction(char*);
int getppid(void);
int yield(void);            // Project 1
int getlev(void);           // Project 1
int set_cpu_share(int);     // Project 1
int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg);  // Project 2
void thread_exit(void *retval);                      // Project 2
int thread_join(thread_t thread, void **retval);     // Project 2
int sync(void);         // Project 3
int get_log_num(void);  // Project 3
int pread(int, void*, int, int);        // Project 3
int pwrite(int, const void*, int, int); // Project 3

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
