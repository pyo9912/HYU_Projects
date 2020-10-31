#include "bpt.h"
#include "file.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Open a table
int open_table(char* pathname) {
    if (pathname == NULL) {
        perror("Failed to get pathname");
    }
    if (strlen(pathname) > 20) {
        perror("Invalid pathname");
    }

    file_open_table(pathname);
}

/*
 * Write all pages of this table from buffer to disk
 */
int close_table(int table_id) {
    if (table_id < 1 || table_id > 10) {
		perror("Invalid table id");
	}

    file_close_table(table_id);
}

/*
 * Find the record containing input ‘key’.
 * If found matching ‘key’, store matched ‘value’ string in ret_val and return 0. 
 * Otherwise, return non-zero value.
 * Memory allocation for record structure(ret_val) should occur in caller function.
 */
int db_find(int table_id, uint64_t key, char* ret_val) {
    record_t* rec;
    buffer_t* buffer_header = get_buffer(table_id, HEADER_NUM);
    page_t* root_page = &buffer_header->frame;
    pagenum_t root_number = ((header_page_t*)root_page)->root_page_number;
    put_buffer(buffer_header);

    rec = find_record(table_id, root_number, key);
    if (rec == NULL) {
        return -1;
    }

    strcpy(ret_val, rec->value);
    return 0;
}

pagenum_t find_leaf_page(int table_id, pagenum_t root, uint64_t key) {
    pagenum_t c = root;
    buffer_t* buffer = NULL;
    page_t* page;
    int idx;
    int cnt = 0;
    //node_page_t* page = (node_page_t*)malloc(PAGESIZE);
    while (c != HEADER_NUM) {
        cnt++;
        if(cnt==3) break;
        buffer = get_buffer(table_id, c);
        page = &buffer->frame;
        if (((node_page_t*)page)->is_leaf == 1) {
            break;
        }

        // Operate in internal page
        if (key < find_key(page, 0)) { 
            // If the key is smaller than the least key in the page
            // Page number should be one_more_page_number
            c = ((node_page_t*)page)->one_more_page_number;
        }
        else {
            // Page number should be rightside of the index
            idx = find_point_index(page, key);
            if (idx == -1) {
                c = HEADER_NUM;
                break;
            }
            c = ((node_page_t*)page)->entries[idx - 1].page_number;
        }
        put_buffer(buffer);
    }
    put_buffer(buffer);

    return c;
}

int find_key_index(page_t* page, uint64_t key) {
    int idx = 0;
    for (idx = 0; idx < ((node_page_t*)page)->number_of_keys; idx++) {
        if (((node_page_t*)page)->records[idx].key == key) {
            return idx;
        }
    }
    return -1;
}

/* Find an index to point */
int find_point_index(page_t* page, uint64_t key) {
    int idx = 0;
    
    if (((node_page_t*)page)->is_leaf == 1) { // Case: leaf page
        while(idx < ((node_page_t*)page)->number_of_keys && ((node_page_t*)page)->records[idx].key < key) {
            idx++;
        }
        return idx;
    }
    else { // Case: internal page
        while(idx < ((node_page_t*)page)->number_of_keys && ((node_page_t*)page)->entries[idx].key < key) {
            idx++;
        }
        return idx;
    }

    return -1;
}

record_t* find_record(int table_id, pagenum_t root, uint64_t key) {
    record_t* rec = NULL;
    page_t* leaf_page;
    pagenum_t c = find_leaf_page(table_id, root, key);
    if (c == HEADER_NUM) {
        return rec;
    }
    buffer_t* buffer = NULL;
    buffer = get_buffer(table_id, c);
    leaf_page = &buffer->frame;

    int idx = find_key_index(leaf_page, key);
    if (idx == -1) {
        put_buffer(buffer);
        return rec;
    }
    rec = (record_t*)malloc(RECORDSIZE);
    rec->key = find_key(leaf_page, idx);
    strcpy(rec->value, find_value(leaf_page, idx));

    put_buffer(buffer);
    return rec;
}

/* Find a key by given index */
uint64_t find_key(page_t* page, int idx) {
    if (((node_page_t*)page)->is_leaf == 1) { // Case: leaf page
        if (idx >= 0 && idx < LEAF_BRANCH_FACTOR - 1) {
            return ((node_page_t*)page)->records[idx].key;
        }
    }
    else { // Case: internal page
        if (idx >= 0 && idx < INTERNAL_BRANCH_FACTOR - 1) {
            return ((node_page_t*)page)->entries[idx].key;
        }
    }

    return -1;
}

/* Find value by given index */
char* find_value(page_t* page, int idx) {
    char* val;
    if (((node_page_t*)page)->is_leaf == 0) return NULL;
    strcpy(val, ((node_page_t*)page)->records[idx].value);
    return val;
}



/*
 * Insert input ‘key/value’ (record) to data file at the right place.
 * If success, return 0. Otherwise, return non-zero value.
 */
int db_insert(int table_id, uint64_t key, char* value) {
    buffer_t* buffer_header = get_buffer(table_id, HEADER_NUM);
    page_t* head = &buffer_header->frame;
    pagenum_t root_number = ((header_page_t*)head)->root_page_number;
    put_buffer(buffer_header);

    root_number = insert(table_id, ((header_page_t*)head)->root_page_number, key, value);
    /* If there is an exist key return -1 */
    if (root_number == -1) {
        //printf("Exist key in tree\n");
        return 0;
    }

    buffer_header = get_buffer(table_id, HEADER_NUM);
    page_t* root_page = &buffer_header->frame;
    buffer_header->is_dirty = 1;
    ((header_page_t*)root_page)->root_page_number = root_number;
    put_buffer(buffer_header);
    
    return 0;
}

pagenum_t insert(int table_id, pagenum_t root, uint64_t key, char* value) {
    record_t* temp;
    buffer_t* buffer;
    page_t* page;
    pagenum_t leaf;

     temp = find_record(table_id, root, key);

     if (temp != NULL) {
         return -1;
     }

    if (root == HEADER_NUM) {
        root = make_root(table_id, key, value);
        return root;
    }
    leaf = find_leaf_page(table_id, root, key);
    if (leaf == HEADER_NUM) {
        return root;
    }

    buffer = get_buffer(table_id, leaf);
    page = &buffer->frame;
    int num_keys = ((node_page_t*)page)->number_of_keys;
    put_buffer(buffer);

    if (num_keys < LEAF_BRANCH_FACTOR - 1) {
        leaf = insert_into_leaf(table_id, leaf, key, value);
        return root;
    }

    //free(page);
    return insert_into_leaf_after_splitting(table_id, root, leaf, key, value);
}

/* Make new leaf page to use */
page_t* make_leaf_page() {
    page_t* leaf_page = (page_t*)malloc(PAGESIZE);
    memset(leaf_page, 0, PAGESIZE);

    ((node_page_t*)leaf_page)->is_leaf = 1;

    return leaf_page;
}


/* Make new internal page to use */
page_t* make_internal_page() {
    page_t* page = (page_t*)malloc(PAGESIZE);
    memset(page, 0, PAGESIZE);
    
    return page;
}

/* Make new root */
pagenum_t make_root(int table_id, uint64_t key, char* value) {
    page_t* root_page = make_leaf_page();

    ((node_page_t*)root_page)->number_of_keys ++;
    strcpy(((node_page_t*)root_page)->records[0].value, value);
    ((node_page_t*)root_page)->records[0].key = key;

    buffer_t* buffer = new_buffer(table_id);
    memcpy(&buffer->frame, root_page, PAGESIZE);

    pagenum_t root_number = buffer->page_num;
    put_buffer(buffer);

    return root_number;
}

/* Case: leaf page can contain more keys */
pagenum_t insert_into_leaf(int table_id, pagenum_t leaf, uint64_t key, char* value) {
    buffer_t* buffer = get_buffer(table_id, leaf);
    page_t* leaf_page = &buffer->frame;
    buffer->is_dirty = 1;
    int i;
    int num_keys = ((node_page_t*)leaf_page)->number_of_keys;
    int insertion_point = find_point_index(leaf_page, key);

    // Shift
    for (i = num_keys; i > insertion_point; i--) {
        ((node_page_t*)leaf_page)->records[i].key = ((node_page_t*)leaf_page)->records[i - 1].key;
        strcpy(((node_page_t*)leaf_page)->records[i].value, ((node_page_t*)leaf_page)->records[i - 1].value);
    }

    // Insert
    ((node_page_t*)leaf_page)->records[insertion_point].key = key;
    strcpy(((node_page_t*)leaf_page)->records[insertion_point].value, value);
    ((node_page_t*)leaf_page)->number_of_keys++;

    put_buffer(buffer);

    return leaf;
}

/* Case: need to balance */
pagenum_t insert_into_leaf_after_splitting(int table_id, pagenum_t root, pagenum_t page_number, uint64_t key, char* value) {
    buffer_t *buffer, *new_leaf_buffer;
    page_t *page, *new_page;
    pagenum_t new_page_number;
    
    buffer = get_buffer(table_id, page_number);
    page = &buffer->frame;
    buffer->is_dirty = 1;

    new_page = make_leaf_page();
    new_leaf_buffer = new_buffer(table_id);
    new_page_number = new_leaf_buffer->page_num;

    int64_t temp_keys[LEAF_BRANCH_FACTOR];
    char temp_values[LEAF_BRANCH_FACTOR][120];

    int i, j, split, num_keys;
    int insertion_point = find_point_index(page, key);

    // Copy to temp space
    for (i = 0, j = 0; i < ((node_page_t*)page)->number_of_keys; i++, j++) {
        if (j == insertion_point) j++;
        temp_keys[j] = ((node_page_t*)page)->records[i].key;
        strcpy(temp_values[j], ((node_page_t*)page)->records[j].value);
    }

    // Insert
    temp_keys[insertion_point] = key;
    strcpy(temp_values[insertion_point], value);

    split = cut(LEAF_BRANCH_FACTOR - 1);

    // Split the records into two leafs
    for (i = 0, num_keys = 0; i < split; i++) {
        ((node_page_t*)page)->records[i].key = temp_keys[i];
        strcpy(((node_page_t*)page)->records[i].value, temp_values[i]);
        ((node_page_t*)page)->number_of_keys = ++num_keys;
    }
    
    for (i = split, j = 0, num_keys = 0; i < LEAF_BRANCH_FACTOR; i++, j++) {
        ((node_page_t*)new_page)->records[j].key = temp_keys[i];
        strcpy(((node_page_t*)new_page)->records[j].value, temp_values[i]);
        ((node_page_t*)new_page)->number_of_keys = ++num_keys;
    }

    // Set new page (right node) a new page number
    ((node_page_t*)new_page)->right_sibling_page_number = ((node_page_t*)page)->right_sibling_page_number;
    ((node_page_t*)page)->right_sibling_page_number = new_page_number;

    for (i = ((node_page_t*)page)->number_of_keys; i < LEAF_BRANCH_FACTOR - 1; i++) {
        ((node_page_t*)page)->records[i].key = 0;
        strcpy(((node_page_t*)page)->records[i].value, "");
    }
    for (i = ((node_page_t*)new_page)->number_of_keys; i < LEAF_BRANCH_FACTOR - 1; i++) {
        ((node_page_t*)new_page)->records[i].key = 0;
        strcpy(((node_page_t*)new_page)->records[i].value, "");
    }

    // Set parent page number
    ((node_page_t*)new_page)->parent_page_number = ((node_page_t*)page)->parent_page_number;

    // Key to push up
    uint64_t up_key = find_key(new_page, 0);

    memcpy(&new_leaf_buffer->frame, new_page, PAGESIZE);

    put_buffer(buffer);
    put_buffer(new_leaf_buffer);

    return insert_into_parent(table_id, root, page_number, up_key, new_page_number);
}

/* Insert key into parent page */
pagenum_t insert_into_parent(int table_id, pagenum_t root, pagenum_t left_number, uint64_t key, pagenum_t right_number) {
    int left_index = 0;
    pagenum_t parent_number;

    buffer_t *left_buffer, *parent_buffer;
    page_t *left_page, *parent_page;
    
    left_buffer = get_buffer(table_id, left_number);
    left_page = &left_buffer->frame;

    parent_number = ((node_page_t*)left_page)->parent_page_number;

    parent_buffer = get_buffer(table_id, parent_number);
    parent_page = &parent_buffer->frame;

    put_buffer(left_buffer);

    // Case: need to make new root
    if(parent_number == HEADER_NUM) {
        return insert_into_new_root(table_id, left_number, key, right_number);
    }

    // Find the parent's pointer to left page
    while (left_index <= ((node_page_t*)parent_page)->number_of_keys && ((node_page_t*)parent_page)->entries[left_index - 1].page_number != left_number) {
        left_index ++;
    }

    if (((node_page_t*)parent_page)->number_of_keys < INTERNAL_BRANCH_FACTOR - 1) {
        return insert_into_node(table_id, root, parent_number, left_index, key, right_number);
    }

    return insert_into_node_after_splitting(table_id, root, parent_number, left_index, key, right_number);
}

/* Insert into internal page */
pagenum_t insert_into_node(int table_id, pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number) {
    buffer_t* parent_buffer;
    page_t* parent_page;

    parent_buffer = get_buffer(table_id, parent_number);
    parent_page = &parent_buffer->frame;
    parent_buffer->is_dirty = 1;

    int i;

    for (i = ((node_page_t*)parent_page)->number_of_keys; i > left_index; i--) {
        ((node_page_t*)parent_page)->entries[i].key = ((node_page_t*)parent_page)->entries[i - 1].key;
        ((node_page_t*)parent_page)->entries[i].page_number = ((node_page_t*)parent_page)->entries[i - 1].page_number;
    }

    ((node_page_t*)parent_page)->entries[left_index].key = key;
    if (left_index == -1) {
        ((node_page_t*)parent_page)->one_more_page_number = right_number;
    }
    else {
        ((node_page_t*)parent_page)->entries[left_index].page_number = right_number;
    }

    ((node_page_t*)parent_page)->number_of_keys++;

    put_buffer(parent_buffer);

    return root;
}

/* Insert into internal page after splitting */
pagenum_t insert_into_node_after_splitting(int table_id, pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number) {
    buffer_t *parent_buffer, *child_buffer, *new_page_buffer;
    page_t *parent_page, *child_page, *new_page;
    pagenum_t new_page_number, child_number;
    
    parent_buffer = get_buffer(table_id, parent_number);
    parent_page = &parent_buffer->frame;
    parent_buffer->is_dirty = 1;
    
    uint64_t up_key;
    uint64_t temp_keys[INTERNAL_BRANCH_FACTOR];
    uint64_t temp_pointers[INTERNAL_BRANCH_FACTOR + 1];

    int i, j, split, num_keys;
    for (i = 0, j = 0; i < ((node_page_t*)parent_page)->number_of_keys + 1; i++, j++) {
        if (j == left_index + 1) {
            j++;
        }
        if (i == 0) {
            temp_pointers[j] = ((node_page_t*)parent_page)->one_more_page_number;
        }
        else {
            temp_pointers[j] = ((node_page_t*)parent_page)->entries[i-1].page_number;
        }
    }

    for (i = 0, j = 0; i < ((node_page_t*)parent_page)->number_of_keys; i++, j++) {
        if (j == left_index) {
            j++;
        }
        temp_keys[j] = ((node_page_t*)parent_page)->entries[i].key;
    }

    temp_pointers[left_index + 1] = right_number;
    temp_keys[left_index] = key;

    split = cut(INTERNAL_BRANCH_FACTOR);
    
    new_page = make_internal_page();
    new_page_buffer = new_buffer(table_id);

    // left internal page
    for (i = 0, num_keys = 0; i < split - 1; i++) {
        if (i == 0) {
            ((node_page_t*)parent_page)->one_more_page_number = temp_pointers[i];
        }
        else {
            ((node_page_t*)parent_page)->entries[i-1].page_number = temp_pointers[i];
        }
        ((node_page_t*)parent_page)->entries[i].key = temp_keys[i];
        num_keys++;
    }
    ((node_page_t*)parent_page)->number_of_keys = num_keys;
    ((node_page_t*)parent_page)->entries[split - 1].page_number = temp_pointers[split - 1];
    up_key = temp_keys[split - 1];

    //right internal page
    for (++i, j = 0, num_keys = 0; i < INTERNAL_BRANCH_FACTOR; i++, j++) {
        if (j == 0) {
            ((node_page_t*)parent_page)->one_more_page_number = temp_pointers[i];
        }
        else {
            ((node_page_t*)parent_page)->entries[j-1].page_number = temp_pointers[i];
        }
        ((node_page_t*)new_page)->entries[j].key = temp_keys[i];
        ++num_keys;
    }
    ((node_page_t*)new_page)->number_of_keys = num_keys;
    ((node_page_t*)new_page)->entries[j].page_number = temp_pointers[i];

    ((node_page_t*)new_page)->parent_page_number = ((node_page_t*)parent_page)->parent_page_number;
    for (i = 0; i <= ((node_page_t*)new_page)->number_of_keys; i++) {
        child_number = ((node_page_t*)new_page)->entries[i].page_number;

        child_buffer = get_buffer(table_id, child_number);
        child_page = &child_buffer->frame;
        child_buffer->is_dirty = 1;

        ((node_page_t*)child_page)->parent_page_number = new_page_number;
        
        put_buffer(child_buffer);
    }
    
    memcpy(&new_page_buffer->frame, new_page, PAGESIZE);

    put_buffer(new_page_buffer);
    put_buffer(parent_buffer);

    return insert_into_parent(table_id, root, parent_number, up_key, new_page_number);
}

/* Case: new root is needed */
pagenum_t insert_into_new_root(int table_id, pagenum_t left_number, uint64_t key, pagenum_t right_number) {
    buffer_t *root_buffer, *left_buffer, *right_buffer;
    page_t *root_page, *left_page, *right_page;

    root_page = make_internal_page();
    root_buffer = new_buffer(table_id);
    pagenum_t root_number = root_buffer->page_num;

    left_buffer = get_buffer(table_id, left_number);
    left_page = &left_buffer->frame;
    left_buffer->is_dirty = 1;

    right_buffer = get_buffer(table_id, right_number);
    right_page = &right_buffer->frame;
    right_buffer->is_dirty = 1;

    ((node_page_t*)root_page)->entries[0].key = key;
    ((node_page_t*)root_page)->one_more_page_number = left_number;
    ((node_page_t*)root_page)->entries[0].page_number = right_number;
    ((node_page_t*)root_page)->number_of_keys = 1;
    ((node_page_t*)root_page)->is_leaf = 0;
    ((node_page_t*)root_page)->parent_page_number = 0;
    ((node_page_t*)left_page)->parent_page_number = root_number;
    ((node_page_t*)right_page)->parent_page_number = root_number;

    memcpy(&root_buffer->frame, root_page, PAGESIZE);

    put_buffer(root_buffer);
    put_buffer(left_buffer);
    put_buffer(right_buffer);

    return root_number;
}

int cut(int length) {
    if (length % 2 == 0) {
        return length / 2;
    }
    else {
        return length / 2 + 1;
    }
}

/*
 * Find the matching record and delete it if found.
 * If success, return 0. Otherwise, return non-zero value.
 */
int db_delete(int table_id, int64_t key) {
    buffer_t* buffer_header = get_buffer(table_id, HEADER_NUM);
    page_t* head = &buffer_header->frame;
    pagenum_t root_number = ((header_page_t*)head)->root_page_number;
    put_buffer(buffer_header);

    root_number = delete(table_id, ((header_page_t*)head)->root_page_number, key);

    if (root_number == -1) {
        return -1;
    }

    buffer_header = get_buffer(table_id, HEADER_NUM);
    head = &buffer_header->frame;
    buffer_header->is_dirty = 1;
    ((header_page_t*)head)->root_page_number = root_number;
    put_buffer(buffer_header);
    return 0;
}

pagenum_t delete(int table_id, pagenum_t root, uint64_t key) {
    pagenum_t leaf_number;
    record_t* rec;

    rec = find_record(table_id, root, key);
    if (rec == NULL) {
        //printf("No matching key in tree\n");
        return -1;
    }

    leaf_number = find_leaf_page(table_id, root, key);

    if (rec != NULL && leaf_number != HEADER_NUM) {
        root = delete_entry(table_id, root, leaf_number, key);
    }

    return root;
}

pagenum_t delete_entry(int table_id, pagenum_t root, pagenum_t leaf_number, uint64_t key) {
    leaf_number = remove_entry_from_node(table_id, leaf_number, key);
    int num_keys;
    buffer_t* leaf_buffer = get_buffer(table_id, leaf_number);
    page_t* leaf_page = &leaf_buffer->frame;
    num_keys = ((node_page_t*)leaf_page)->number_of_keys;
    put_buffer(leaf_buffer);

    // Case: deletion from root
    if (leaf_number == root) {
        return adjust_root(table_id, root);
    }

    // Case: deletion from non-root
    // Delayed Merge
    if (num_keys <= 0) {
        root = coalesce_nodes(table_id, root, leaf_number);
    }

    return root;
}

pagenum_t remove_entry_from_node(int table_id, pagenum_t leaf_number, uint64_t key) {
    buffer_t* leaf_buffer = get_buffer(table_id, leaf_number);
    page_t* leaf = &leaf_buffer->frame;
    leaf_buffer->is_dirty = 1;
    int i, j;

    i = find_key_index(leaf, key);

    // Remove record in leaf page
    for (j = i; j < ((node_page_t*)leaf)->number_of_keys; j++) {
        ((node_page_t*)leaf)->records[j].key = ((node_page_t*)leaf)->records[j + 1].key;
        strcpy(((node_page_t*)leaf)->records[j].value, ((node_page_t*)leaf)->records[j + 1].value);
    }

    ((node_page_t*)leaf)->number_of_keys--;
    ((node_page_t*)leaf)->records[((node_page_t*)leaf)->number_of_keys].key = 0;
    strcpy(((node_page_t*)leaf)->records[((node_page_t*)leaf)->number_of_keys].value, "");

    put_buffer(leaf_buffer);

    return leaf_number;
}

pagenum_t adjust_root(int table_id, pagenum_t root_number) {
    buffer_t* root_buffer = get_buffer(table_id, root_number);
    page_t* root = &root_buffer->frame;
    
    pagenum_t new_root_number;

    if (((node_page_t*)root)->number_of_keys > 0) {
        put_buffer(root_buffer);
        return root_number;
    }

    if (((node_page_t*)root)->is_leaf == 0) {
        new_root_number = ((node_page_t*)root)->one_more_page_number;
        buffer_t* new_buffer = get_buffer(table_id, new_root_number);
        page_t* new_root = &new_buffer->frame;
        new_buffer->is_dirty = 1;
        ((node_page_t*)new_root)->parent_page_number = 0;
        put_buffer(new_buffer);
    }

    else {
        new_root_number = HEADER_NUM;
    }
    free_buffer(table_id, root_buffer);
    put_buffer(root_buffer);

    return new_root_number;
}

pagenum_t get_neighbor_number(int table_id, pagenum_t n) {
    pagenum_t c = n;
    pagenum_t neighbor_pagenum = HEADER_NUM;
    buffer_t* buffer;
    buffer = get_buffer(table_id, c);
    page_t* page;
    page = &buffer->frame;

    c = ((node_page_t*)page)->parent_page_number;
    put_buffer(buffer);

    buffer = get_buffer(table_id, c);
    page = &buffer->frame;

    int i;
    pagenum_t temp;
    for (i = 0; i <= ((node_page_t*)page)->number_of_keys; i++) {
        if (i == 0) {
            temp = ((node_page_t*)page)->one_more_page_number;
        }
        else {
            temp = ((node_page_t*)page)->entries[i - 1].page_number;
        }
        if (temp == n) {
            if (i > 0) {
                // Case: page has left neighbor
                neighbor_pagenum = ((node_page_t*)page)->entries[i - 1].page_number;
                break;
            }
            else {
                // Case: page is left most child
                
                neighbor_pagenum = ((node_page_t*)page)->entries[0].page_number;
                break;
            }
        }
    }
    
    return neighbor_pagenum;
}

pagenum_t coalesce_nodes(int table_id, pagenum_t root, pagenum_t page_number_to_free) {
    pagenum_t parent_number, neighbor_number, child_number;

    buffer_t* to_free_buffer = get_buffer(table_id, page_number_to_free);
    page_t* free_page = &to_free_buffer->frame;
    to_free_buffer->is_dirty = 1;

    int i, j;

    parent_number = ((node_page_t*)free_page)->parent_page_number;
    
    if (parent_number == HEADER_NUM) {
        put_buffer(to_free_buffer);
        return adjust_root(table_id, root);
    }    
    
    buffer_t* parent_buffer = get_buffer(table_id, parent_number);
    page_t* parent_page = &parent_buffer->frame;
    parent_buffer->is_dirty = 1;

    if (((node_page_t*)free_page)->is_leaf == 1) { // Case: leaf page
        // Append free_page's entries on neighbor_page
        neighbor_number = get_neighbor_number(table_id, page_number_to_free);
        buffer_t* neighbor_buffer;
        neighbor_buffer = get_buffer(table_id, neighbor_number);
        page_t* neighbor_page;
        neighbor_page = &neighbor_buffer->frame;
        neighbor_buffer->is_dirty = 1;
    
        int neighbor_insertion_index = ((node_page_t*)neighbor_page)->number_of_keys;

        for (i = neighbor_insertion_index, j = 0; j < ((node_page_t*)free_page)->number_of_keys; i++, j++) {
            ((node_page_t*)neighbor_page)->records[i].key = ((node_page_t*)free_page)->records[j].key;
            strcpy(((node_page_t*)neighbor_page)->records[i].value, ((node_page_t*)free_page)->records[j].value);
        }

        // Move the right sibling page number
        ((node_page_t*)neighbor_page)->right_sibling_page_number = ((node_page_t*)free_page)->right_sibling_page_number;

        put_buffer(neighbor_buffer);

        // Remove the page from parent
        pagenum_t temp;
        for (i = 0; i < ((node_page_t*)parent_page)->number_of_keys; i++) {
            if (i == 0) {
                temp = ((node_page_t*)parent_page)->one_more_page_number;
            }
            else {
                temp = ((node_page_t*)parent_page)->entries[i - 1].page_number;
            }
            if (page_number_to_free == temp) {
                break;
            }
        }
        for (j = i + 1; j < ((node_page_t*)parent_page)->number_of_keys; j++) {
            if (i != 0) {
                ((node_page_t*)parent_page)->entries[j - 1].key = ((node_page_t*)parent_page)->entries[j].key;
            }
            ((node_page_t*)parent_page)->entries[j].page_number = ((node_page_t*)parent_page)->entries[j + 1].page_number;
        }
        // Clear last index page
        ((node_page_t*)parent_page)->entries[j - 1].key = 0;
        ((node_page_t*)parent_page)->entries[j].page_number = 0;
        ((node_page_t*)parent_page)->number_of_keys--;
    }
    else { // Case: internal page
        // Get the number of only child of the page to free
        child_number = ((node_page_t*)free_page)->one_more_page_number;
        buffer_t* child_buffer;
        child_buffer = get_buffer(table_id, child_number);
        page_t* child_page;
        child_page = &child_buffer->frame;
        child_buffer->is_dirty = 1;

        // Search index of the parent which point the page to free
        for (i = 0; i < ((node_page_t*)parent_page)->number_of_keys; i++) {
            if (page_number_to_free == ((node_page_t*)parent_page)->entries[i].page_number) {
                break;
            }
        }

        // Connect free_page's child to free_page's parent
        if (i != 0) {
            ((node_page_t*)parent_page)->entries[i - 1].key = ((node_page_t*)parent_page)->entries[i].key;
        }
        ((node_page_t*)parent_page)->entries[i].page_number = child_number;
        ((node_page_t*)child_page)->parent_page_number = parent_number;

        put_buffer(child_buffer);
    }
    int parent_key_num = ((node_page_t*)parent_page)->number_of_keys;
    put_buffer(parent_buffer);

    // Call coalesce_nodes() recursively when parent_page becomes empty
    if (parent_key_num <= 0) {
        root = coalesce_nodes(table_id, root, parent_number);
    }

    free_buffer(table_id, to_free_buffer);
    put_buffer(to_free_buffer);

    return root;
}


/*
 * Print functions
 */


node_t* Q;

void enqueue(pagenum_t new_node_number, int level) {
    node_t* c  = (node_t*)malloc(sizeof(node_t));
    node_t* new_node  = (node_t*)malloc(sizeof(node_t));

    new_node->page_number = new_node_number;
    new_node->level = level;
    new_node->next = NULL;

    if (Q == NULL) {
        Q = new_node;
    }
    else {
        c = Q;
        while (c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
    }
}

pagenum_t dequeue(int* level) {
    if (Q == NULL) {
        return HEADER_NUM;
    }
    pagenum_t page_num = Q->page_number;
    node_t* node = Q;
    Q = Q->next;
    *level = node->level;
    node->next = NULL;
    free(node);
    return page_num;
}

void print_tree(int table_id) {
    buffer_t* buffer_header = get_buffer(table_id, HEADER_NUM);
    page_t* head = &buffer_header->frame;
    pagenum_t root = ((header_page_t*)head)->root_page_number;
    put_buffer(buffer_header);

    if (root == HEADER_NUM) {
        printf("Empty tree.\n");
        return;
    }

    buffer_t* buffer;
    page_t* page;
    pagenum_t c;
    pagenum_t temp;
    int i;
    int current_level = 0;
    int node_level;
    Q = NULL;
    enqueue(root, current_level);

    while (Q != NULL) {
        c = dequeue(&node_level);
        buffer = get_buffer(table_id, c);
        page = &buffer->frame;

        if (current_level < node_level) {
            printf("\n");
            current_level = node_level;
        }

        for (i = 0; i < ((node_page_t*)page)->number_of_keys; i++) {
            printf("{ %lu }", find_key(page, i));
        }
        if (((node_page_t*)page)->is_leaf == 0) {
            for (i = 0; i <= ((node_page_t*)page)->number_of_keys; i++) {
                if (i == 0) {
                    temp = ((node_page_t*)page)->one_more_page_number;
                }
                else {
                    temp = ((node_page_t*)page)->entries[i-1].page_number;
                }
                enqueue(temp, node_level + 1);
            }
        }
        printf(" | ");
        put_buffer(buffer);
    }
    printf("\n");
}
