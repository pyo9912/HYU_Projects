#ifndef __FILE_H__
#define __FILE_H__

#include "bpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

typedef uint64_t pagenum_t;

/*structure definition*/

// Record structure (key+value)
typedef struct record {
	uint64_t key;
	char value[120];
} record_t;

// Entry structure (key+page_number)
typedef struct entry {
	uint64_t key;
	pagenum_t page_number;
} entry_t;

// Node structure for queue
typedef struct node {
    pagenum_t page_number;
    struct node * next;
} node_t;

// Header page structure
typedef struct header_page_t {
	uint64_t free_page_number;
	uint64_t root_page_number;
	uint64_t number_of_pages;
	char reserved[4072];
} header_page_t;

// Free page structure
typedef struct free_page_t {
	uint64_t next_free_page_number;
	char not_used[4088];
} free_page_t;

typedef union node_page_t {
	uint64_t parent_page_number;
	uint32_t is_leaf;
	uint32_t number_of_keys;
	char reserved[104];
	union {
		uint64_t right_sibling_page_number; // Leaf page
		uint64_t one_more_page_number; // Internal page
	};
	union {
		record_t records[31]; // Leaf page
		entry_t entries[248]; // Internal page
	};
} node_page_t;

// in-memory page union
typedef union page_t {
	header_page_t header_page;
	free_page_t free_page;
	node_page_t node_page;
} page_t;

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page();

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src);


#endif /*__FILE_H__*/