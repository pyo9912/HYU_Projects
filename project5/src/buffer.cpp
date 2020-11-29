#include "buffer.h"
#include "file.h"
#include "bpt.h"
#include "trx.h"
#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t buffer_manager_latch;

// For LRU
static buffer_t* pool = NULL;

/*
 * Initialize the buffer pool with the given number
 */
int init_db(int buf_num) {
    if (buf_num < 0) {
        return -1;
    }
    buffer_manager_latch = PTHREAD_MUTEX_INITIALIZER;
    init_lock_table();
    pthread_mutex_lock(&buffer_manager_latch);
    buffer_t* temp = NULL;
    buffer_t* prev = NULL;

    // Use circular double-linked list
    while (buf_num != 0) {
        temp = (buffer_t*)malloc(sizeof(buffer_t));
        if (temp == NULL) {
            perror("Failed to allocate buffer");
        }
        memset(&temp->frame, 0, PAGESIZE);
        temp->table_id = 0;
        temp->page_num = 0;
        temp->is_dirty = 0;
        temp->is_lock = 0;
        temp->page_latch = PTHREAD_MUTEX_INITIALIZER;

        if (pool == NULL) {
            pool = temp;
            prev = pool;
        }
        else {
            temp->prev = prev;
            temp->prev->next = temp;
            prev = temp;
        }
        buf_num--;
    }
    // Make circular
    temp->next = pool;
    temp->next->prev = temp;
    pthread_mutex_unlock(&buffer_manager_latch);
    return 0;
}

/*
 * Flush all data from buffer and destroy allocated buffer
 */
int shutdown_db(void) {
    pthread_mutex_lock(&buffer_manager_latch);

    if (pool == NULL) {
        // No data in buffer
        return 0;
    }
    buffer_t* cur;
    buffer_t* next;

    cur = pool;
    next = cur->next;

    while (next != pool) {
        //flush
        flush_buffer(cur);
        //free
        free(cur);
        cur = next;
        next = cur->next;
    }

    pthread_mutex_unlock(&buffer_manager_latch);
    return 0;
}

// Flush the buffers of the table
void flush_buffer(buffer_t* buffer) {
    if (buffer == NULL) {
        return;
    }
    // Lock buffer block
    pthread_mutex_lock(&buffer->page_latch);
    buffer->is_lock = 1;
    if (buffer->is_dirty == 1) {
        page_t* page;
        // Get page of buffer
        page = &(buffer->frame);
        // Write page on disk
        file_write_page(buffer->table_id, buffer->page_num, page);
        buffer->is_dirty = 0;
    }
    // Clear lock
    buffer->is_lock = 0;
    pthread_mutex_unlock(&buffer->page_latch);
}

void close_buffer_table(int table_id) {
    pthread_mutex_lock(&buffer_manager_latch);
    buffer_t* cur = pool;
    while (cur != NULL) {
        if (cur->table_id == table_id) {
            // Case: page is dirty
            flush_buffer(cur);
            memset(&(cur->frame), 0, PAGESIZE);
            cur->table_id = 0;
            cur->page_num = 0;
        }
        cur = cur->next;
        if (cur == pool) {
            break;
        }
    }
    pthread_mutex_unlock(&buffer_manager_latch);
}

// Move buffer to pool->next
void put_buffer(buffer_t* buffer) {
    if (buffer == NULL) {
        return;
    }
    pthread_mutex_lock(&buffer_manager_latch);
    if (buffer == pool) {
        pool = pool->prev;
    }
    else {
        buffer->prev->next = buffer->next;
        buffer->next->prev = buffer->prev;
        buffer->prev = pool;
        buffer->next = pool->next;
        buffer->prev->next = buffer;
        buffer->next->prev = buffer;
    }
    buffer->is_lock = 0;
    pthread_mutex_unlock(&buffer->page_latch);
    pthread_mutex_unlock(&buffer_manager_latch);
}

buffer_t* find_buffer(int table_id, pagenum_t pagenum) {
    buffer_t* temp = pool->prev;
    // Case: Can't find specific page in specific table
    while (temp != NULL) {
        if (temp == pool) {
            // Case: No buffer is available
            return NULL;
        }
        if (temp->is_lock == 0) {
            // Get victim
            flush_buffer(temp);
            temp->table_id = table_id;
            temp->page_num = pagenum;
            memset(&temp->frame, 0, PAGESIZE);
            break;
        }
        temp = temp->prev;
    }
    return temp;
}

// Read buffer control block from buffer pool
buffer_t* get_buffer(int table_id, pagenum_t pagenum) {
    pthread_mutex_lock(&buffer_manager_latch);
    buffer_t* temp = pool->next;
    while (temp != NULL) {
        if (temp == pool) {
            break;
        }
        if (temp->table_id == table_id && temp->page_num == pagenum) {
            // Lock buffer block
            pthread_mutex_lock(&temp->page_latch);
            temp->is_lock = 1;
            pthread_mutex_unlock(&buffer_manager_latch);
            return temp;
        }
        temp = temp->next;
    }
    temp = find_buffer(table_id, pagenum);
    // Lock
    pthread_mutex_lock(&temp->page_latch);
    // Read page
    file_read_page(temp->table_id, temp->page_num, &(temp->frame));
    temp->is_lock = 1;
    pthread_mutex_unlock(&buffer_manager_latch);
    return temp;
}

buffer_t* new_buffer(int table_id) {
    // For allocate new page
    pagenum_t pagenum = file_alloc_page(table_id);
    buffer_t* head = get_buffer(table_id, HEADER_NUM);
    // Set header
    ((header_page_t*)&(head->frame))->number_of_pages++;
    head->is_dirty = 1;
    put_buffer(head);

    buffer_t* buffer = get_buffer(table_id, pagenum);
    memset(&buffer->frame, 0, PAGESIZE);
    buffer->table_id = table_id;
    buffer->page_num = pagenum;
    buffer->is_dirty = 0;
    buffer->is_lock = 0;
    buffer->page_latch = PTHREAD_MUTEX_INITIALIZER;

    return buffer;
}

void free_buffer(int table_id, buffer_t* buffer) {
    // For free page
    file_free_page(table_id, buffer->page_num);
    buffer->is_dirty = 0;
    // Unlink buffer
    buffer->prev->next = buffer->next;
    buffer->next->prev = buffer->prev;
    if (buffer == pool) {
        pool = buffer->next;
    }
}