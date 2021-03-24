#include "types.h"
#include "defs.h"

// Simple system call
int
printk_str(char *str)
{
    cprintf("%s\n", str);
    return 0xABCDABCD;
}

// Wrapper for my_syscall
int
sys_myfunction(void)
{
    char *str;
    // Decode argument using argstr
    // argstr: Get parameter from user stack while mode switching
    if (argstr(0, &str) < 0)
    {
        return -1;
    }
    return printk_str(str);
}