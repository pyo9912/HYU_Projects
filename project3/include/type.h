#ifndef __TYPE__H__
#define __TYPE__H__

#define PAGESIZE 4096
#define RECORDSIZE 128
#define LEAF_BRANCH_FACTOR 32
#define INTERNAL_BRANCH_FACTOR 249
#define HEADER_NUM 0
#define TABLE_SIZE 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

typedef uint64_t pagenum_t;
typedef uint64_t offset_t;

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
	int level;
    struct node * next;
} node_t;

// Header page structure
typedef struct header_page {
	uint64_t free_page_number;
	uint64_t root_page_number;
	uint64_t number_of_pages;
	char reserved[4072];
} header_page_t;

// Free page structure
typedef struct free_page {
	uint64_t next_free_page_number;
	char not_used[4088];
} free_page_t;

typedef struct node_page {
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
typedef union page {
	header_page_t header_page;
	free_page_t free_page;
	node_page_t node_page;
} page_t;

typedef struct buffer {
    page_t frame;
    int table_id;
    pagenum_t page_num;
    int is_dirty;
    int is_pinned;
    struct buffer *next;
    struct buffer *prev;
} buffer_t;

#endif