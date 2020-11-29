#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "type.h"

// Initialize buffer pool with num_buf
int init_db(int buf_num);
// Destroy buffer manager
int shutdown_db(void);
// Flush data into disk
void flush_buffer(buffer_t* buffer);
// Close buffer table
void close_buffer_table(int table_id);
// Move buffer to pool->next
void put_buffer(buffer_t* buffer);
// Find available buffer block
buffer_t* find_buffer(int table_id, pagenum_t pagenum);
// Read the buffer control block from buffer pool
buffer_t* get_buffer(int table_id, pagenum_t pagenum);
// For allocations
buffer_t* new_buffer(int table_id);
// For free
void free_buffer(int table_id, buffer_t* buffer);

#endif /* __BUFFER_H_ */