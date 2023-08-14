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

/**
 * This function requests portion of CPU resources with given parameter
 * value by calling set_cpu_share() system call.
 * It reports the cnt value which have been accumulated during LIFETIME.
 */
int
main(int argc, char *argv[])
{
    int i;
    int cnt = 0;
    int cpu_share;
    int start_tick;
    int curr_tick;

    if (argc < 2) {
        printf(1, "usage: 'test_stride' + (portion of CPU share)\n");
        exit();
    }

    cpu_share = atoi(argv[1]);

    if (set_cpu_share(cpu_share) < 0) {
        printf(1, "Error: Cannot set cpu share\n");
        exit();
    } 

    start_tick = uptime();

    i = 0;
    for(;;) {
        i++;
        if (i == COUNT_PERIOD) {
            cnt++;
            curr_tick = uptime();

            if(curr_tick - start_tick > LIFETIME) {
                printf(1, "STRIDE(%d%%), cnt: %d\n", cpu_share, cnt);
                break;
            }
            i = 0;
        }
    }
    exit();
}