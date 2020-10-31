#ifndef __BPT_H__
#define __BPT_H__

#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Open table
int open_table(char* pathname);
// Close table
int close_table(int table_id); 
// find
int db_find(int table_id, uint64_t key, char* ret_val);
// insert
int db_insert(int table_id, uint64_t key, char* value);
// delete
int db_delete(int table_id, int64_t key);

/* Find */
pagenum_t find_leaf_page(int table_id, pagenum_t root, uint64_t key);
record_t* find_record(int table_id, pagenum_t root, uint64_t key);
int find_key_index(page_t* page, uint64_t key);
int find_point_index(page_t* page, uint64_t key);
uint64_t find_key(page_t* page, int idx);
char* find_value(page_t* page, int idx);


/* Insert */
pagenum_t insert(int table_id, pagenum_t root, uint64_t key, char* value);
page_t* make_leaf_page();
page_t* make_internal_page();
pagenum_t make_root(int table_id, uint64_t key, char* value);
pagenum_t insert_into_leaf(int table_id, pagenum_t leaf, uint64_t key, char* value);
pagenum_t insert_into_leaf_after_splitting(int table_id, pagenum_t root, pagenum_t page_number, uint64_t key, char* value);
pagenum_t insert_into_parent(int table_id, pagenum_t root, pagenum_t left_number, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_node(int table_id, pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_node_after_splitting(int table_id, pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_new_root(int table_id, pagenum_t left_number, uint64_t key, pagenum_t right_number);
int cut(int length);


/* Delete */
pagenum_t delete(int table_id, pagenum_t root, uint64_t key);
pagenum_t delete_entry(int table_id, pagenum_t root, pagenum_t leaf_number, uint64_t key);
pagenum_t remove_entry_from_node(int table_id, pagenum_t leaf_number, uint64_t key);
pagenum_t adjust_root(int table_id, pagenum_t root_number);
pagenum_t get_neighbor_number(int table_id, pagenum_t n);
pagenum_t coalesce_nodes(int table_id, pagenum_t root, pagenum_t page_number_to_free);


/* Print */

void enqueue(pagenum_t new_node_number, int level);
pagenum_t dequeue();
void print_tree(int table_id);


#endif /*__BPT_H__*/