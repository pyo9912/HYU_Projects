/**
 * This program runs various workloads cuncurrently.
 */


#include "types.h"
#include "stat.h"
#include "user.h"

#define LIFETIME    (1000) /* (ticks) */
#define COUNT_PERIOD (1000000) /* (iteration) */

#define MLFQ_LEVEL  (3) /* Number of level(priority) of MLFQ scheduler */

#define WORKLOAD_NUM (10) /* The number of workloads */

enum { MLFQ_NONE, MLFQ_LEVCNT, MLFQ_YIELD, MLFQ_LEVCNT_YIELD };
void
test_mlfq(int type)
{
    int cnt = 0;
    int i = 0;
    int start_tick;
    int curr_tick;

    /* Get start tick */
    start_tick = uptime();

    for (;;) {
        i++;
        if (i >= COUNT_PERIOD) {
            cnt++;
            i = 0;

            /* Get current tick */
            curr_tick = uptime();

            if (curr_tick - start_tick > LIFETIME) {
                /* Time to terminate */
                break;
            }

            if (type == MLFQ_YIELD || type == MLFQ_LEVCNT_YIELD) {
                /* Yield process itself, not by timer interrupt */
            }
        }
    }
    printf(1, "MLfQ(%s, %d), cnt : %d\n",
    type == MLFQ_NONE ? "compute" : "yield", getpid(), cnt);


    return;
}

struct workload {
    void (*func)(int);
    int arg;
};

int
main(int argc, char *argv[])
{
    int pid;
    int i;

    /* Workload list */
    struct workload workloads[WORKLOAD_NUM] = {
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
        {test_mlfq, MLFQ_NONE},
    };

    for (i = 0; i < WORKLOAD_NUM; i++) {
        pid = fork();
        if (pid > 0) {
            /* Parent */
            continue;
        } else if (pid == 0) {
            /* Child */
            void (*func)(int) = workloads[i].func;
            int arg = workloads[i].arg;
            /* Do this workload */
            func(arg);
            exit();
        } else {
            printf(1, "FAIL : fork\n");
            exit();
        }
    }

    for (i = 0; i < WORKLOAD_NUM; i++) {
        wait();
    }

    exit();
}