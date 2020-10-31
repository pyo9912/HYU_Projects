#include "buffer.h"
#include "file.h"
#include "bpt.h"

#include <stdio.h>
#include <stdlib.h>

// For LRU
static buffer_t* pool = NULL;

/*
 * Initialize the buffer pool with the given number
 */
int init_db(int buf_num) {
    if (buf_num < 0) {
        return -1;
    }
    buffer_t* temp = NULL;
    buffer_t* prev = NULL;

    // Use circular double-linked list
    while (buf_num != 0) {
        temp = (buffer_t*)malloc(sizeof(buffer_t));
        if (temp == NULL) {
            perror("Failed to allocate buffer");
        }
        
        temp->table_id = 0;
        temp->page_num = 0;
        temp->is_dirty = 0;
        temp->is_pinned = 0;

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

    return 0;
}

/*
 * Flush all data from buffer and destroy allocated buffer
 */
int shutdown_db(void) {
    if (pool == NULL) {
        // No data in buffer
        return 0;
    }
    buffer_t* this;
    buffer_t* next;

    this = pool;
    next = this->next;

    while (next != pool) {
        //flush
        flush_buffer(this);
        //free
        free(this);
        this = next;
        next = this->next;
    }

    return 0;
}

// Flush the buffers of the table
void flush_buffer(buffer_t* buffer) {
    if (buffer == NULL) {
        return;
    }
    // Pin buffer block
    buffer->is_pinned += 1;
    
    if (buffer->is_dirty == 1) {
        page_t* page;
        // Get page of buffer
        page = &(buffer->frame);
        // Write page on disk
        file_write_page(buffer->table_id, buffer->page_num, page);
        buffer->is_dirty = 0;
    }
    // Clear pin
    buffer->is_pinned -= 1;
}

void close_buffer_table(int table_id) {
    buffer_t* this = pool;
    while (this != NULL) {
        if (this->table_id == table_id) {
            // Case: page is dirty
            flush_buffer(this);
            memset(&(this->frame), 0, PAGESIZE);
            this->table_id = 0;
            this->page_num = 0;
        }
        this = this->next;
        if (this == pool) {
            break;
        }
    }
}

// Move buffer to pool->next
void put_buffer(buffer_t* buffer) {
    if (buffer == NULL) {
        return;
    }
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
    buffer->is_pinned -= 1;
}

buffer_t* find_buffer(int table_id, pagenum_t pagenum) {
    buffer_t* temp = pool->prev;

    // Case: Can't find specific page in specific table
    while (temp != NULL) {
        if (temp == pool) {
            // Case: No buffer is available
            return NULL;
        }
        if (temp->is_pinned == 0) {
            // Get victim
            flush_buffer(temp);
            temp->table_id = table_id;
            temp->page_num = pagenum;
            break;
        }
        temp = temp->prev;
    }
    return temp;
}

// Read buffer control block from buffer pool
buffer_t* get_buffer(int table_id, pagenum_t pagenum) {
    buffer_t* temp = pool->next;

    while (temp != NULL) {
        if (temp == pool) {
            break;
        }
        if (temp->table_id == table_id && temp->page_num == pagenum) {
            // Pin buffer block
            temp->is_pinned += 1;
            return temp;
        }
        temp = temp->next;
    }

    temp = find_buffer(table_id, pagenum);

    // Read page
    file_read_page(temp->table_id, temp->page_num, &(temp->frame));
    // Pin
    temp->is_pinned += 1;

    return temp;
}

buffer_t* new_buffer(int table_id) {
    // For allocte new page
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
    buffer->is_pinned = 0;

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