#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <pthread.h>
#include <stdint.h>

typedef struct lock_entry_t lock_entry_t;
typedef struct lock_t lock_t;

enum lock_mode_t {
	ACQUIRED, WAITING
};

struct lock_entry_t {
	int table_id;
	int64_t record_id;
	lock_t *tail, *head;
};

struct lock_t {
	lock_mode_t mode;
	pthread_cond_t cond;
    lock_entry_t *sentinel;
	lock_t *prev, *next;
};

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
