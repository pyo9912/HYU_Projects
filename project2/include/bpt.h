#ifndef __BPT_H__
#define __BPT_H__

#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PAGESIZE 4096
#define LEAF_BRANCH_FACTOR 32
#define INTERNAL_BRANCH_FACTOR 249
#define STARTNUM 0

/* db_API */
// Open a table
int open_table(char* pathname);
// find
int db_find(uint64_t key, char* ret_val);
// insert
int db_insert(uint64_t key, char* value);
// delete
int db_delete(int64_t key);


/* Find */
pagenum_t find_leaf_page(pagenum_t root, uint64_t key);
int find_key_index(page_t* page, uint64_t key);
uint64_t find_key(page_t* page, int idx);
char* find_value(page_t* page, int idx);


/* Insert */
pagenum_t insert(pagenum_t root, uint64_t key, char* value);
pagenum_t make_page();
pagenum_t make_root(uint64_t key, char* value);
pagenum_t insert_into_leaf(pagenum_t leaf, uint64_t key, char* value);
pagenum_t insert_into_leaf_after_splitting(pagenum_t root, pagenum_t page_number, uint64_t key, char* value);
pagenum_t insert_into_parent(pagenum_t root, pagenum_t left_number, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_node(pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_node_after_splitting(pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number);
pagenum_t insert_into_new_root(pagenum_t left_number, uint64_t key, pagenum_t right_number);
int cut(int length);


/* Delete */
pagenum_t delete(pagenum_t root, uint64_t key);
pagenum_t delete_entry(pagenum_t root, pagenum_t leaf_number, uint64_t key);
pagenum_t remove_entry_from_node(pagenum_t leaf_number, uint64_t key);
pagenum_t adjust_root(pagenum_t root_number);
pagenum_t get_neighbor_number(pagenum_t n);
pagenum_t coalesce_nodes(pagenum_t root, pagenum_t page_number_to_free);


/* Print */
/*
void enqueue(pagenum_t new_node_number);
pagenum_t dequeue();
void print_leaves(pagenum_t root);
void print_tree (pagenum_t root);
int path_to_root(pagenum_t root, pagenum_t page_number);
*/


#endif /*__BPT_H__*/