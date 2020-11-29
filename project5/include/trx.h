#ifndef __TRX_H__
#define __TRX_H__

#include "type.h"

/* APIs for lock table */
int trx_begin();
int trx_commit(int trx_id);
void trx_release(trx_t* trx);

int lock_release_all(int trx_id);
void lock_remove_trx(int trx_id);

int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode);
int lock_release(lock_t* lock_obj);
trx_t* get_trx_block(int trx_id);

#endif /* __TRX_H__ */