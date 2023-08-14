#include "bpt.h"
#include "file.h"

// GLOBALS
extern int fd;

extern header_page_t header;

node_t* Q = NULL;

// Open a table
int open_table(char* pathname) {
    if (pathname == NULL) {
        perror("Failed to get pathname");
    }
	fd = open(pathname, O_CREAT | O_RDWR | O_SYNC, 0777);
	if (fd == -1) {
		perror("Failed to load page");
	}

	//header_page_t* header = (header_page_t*)malloc(PAGESIZE);
	if (&header == NULL) {
		perror("Failed to allocate header");
	}
    file_read_page(0, &header);
    memset(&header, 0, PAGESIZE);
    (&header)->number_of_pages = 1;
    file_write_page(0, &header);

	return fd;
}

/*
 * Find the record containing input ‘key’.
 * If found matching ‘key’, store matched ‘value’ string in ret_val and return 0. 
 * Otherwise, return non-zero value.
 * Memory allocation for record structure(ret_val) should occur in caller function.
 */
int db_find(uint64_t key, char* ret_val) {
    pagenum_t root;
    pagenum_t leaf_page_number;
    int idx;

    //header_page_t* header = (header_page_t*)malloc(PAGESIZE);
    file_read_page(0, (page_t*)&header);

    root = (&header)->root_page_number;
    leaf_page_number = find_leaf_page(root, key);

    page_t leaf;
    //node_page_t* leaf = (node_page_t*)malloc(PAGESIZE);
    file_read_page(leaf_page_number, &leaf);

    idx = find_key_index(&leaf, key);
    if (idx == -1) return -1;
    
    strcpy(ret_val, find_value(&leaf, idx));

    //free(leaf);
    //free(header);
    return 0;
}

pagenum_t find_leaf_page(pagenum_t root, uint64_t key) {
    pagenum_t c = root;
    int idx;
    page_t page;
    //node_page_t* page = (node_page_t*)malloc(PAGESIZE);
    while (c != 0) {
        file_read_page((c), &page);
        if (((node_page_t*)&page)->is_leaf != 0) break;
        // Operate in internal page
        idx = find_key_index(&page, key);
        if (idx == 0) { 
            // If the key is smaller than the least key in the page
            // Page number should be one_more_page_number
            c = ((node_page_t*)&page)->one_more_page_number;
        }
        else {
            // Page number should be rightside of the index
            c = ((node_page_t*)&page)->entries[idx].page_number;
        }
    }
    //free(page);
    return c;
}

/* Find an index to point */
int find_key_index(page_t* page, uint64_t key) {
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
    char val[120];
    if (((node_page_t*)page)->is_leaf == 0) return NULL;
    strcpy(val, ((node_page_t*)page)->records[idx].value);
    return val;
}

/*
 * Insert input ‘key/value’ (record) to data file at the right place.
 * If success, return 0. Otherwise, return non-zero value.
 */
int db_insert(uint64_t key, char* value) {
    pagenum_t root;

    //header_page_t* header = (header_page_t*)malloc(PAGESIZE);
    file_read_page(0, &header);

    root = insert((&header)->root_page_number, key, value);
    (&header)->root_page_number = root;

    file_write_page(0, &header);
    //free(header);
    return 0;
}

pagenum_t insert(pagenum_t root, uint64_t key, char* value) {
    page_t page;
    pagenum_t leaf;
    //page_t* page = (page_t*)malloc(PAGESIZE);

    if (root == 0) {
        root = make_root(key, value);
        return root;
    }
    leaf = find_leaf_page(root, key);
    if (leaf == 0) return root;

    file_read_page((leaf), &page);
    if (((node_page_t*)&page)->number_of_keys < LEAF_BRANCH_FACTOR - 1) {
        leaf = insert_into_leaf(leaf, key, value);
        file_write_page((leaf), &page);
        return root;
    }

    //free(page);
    return insert_into_leaf_after_splitting(root, leaf, key, value);
}

/* Make new page to use */
pagenum_t make_page() {
    page_t new_page;
    //page_t* new_page = (page_t*)malloc(PAGESIZE);
    pagenum_t new_page_number = file_alloc_page();

    file_read_page(new_page_number, &new_page);
    memset(&new_page, 0, PAGESIZE);
    ((node_page_t*)&new_page)->number_of_keys = 1;

    file_write_page(new_page_number, &new_page);

    //free(new_page);
    return new_page_number;
}

/* Make new root */
pagenum_t make_root(uint64_t key, char* value) {
    pagenum_t root = make_page();
    page_t page;
    //page_t* page = (page_t*)malloc(PAGESIZE);

    file_read_page((root), &page);
    
    
    strcpy(((node_page_t*)&page)->records[0].value, value); 
    ((node_page_t*)&page)->number_of_keys = 1;
    ((node_page_t*)&page)->records[0].key = key;

    file_write_page((root), &page);
    //free(page);
    return root;
}

/* Case: leaf page can contain more keys */
pagenum_t insert_into_leaf(pagenum_t leaf, uint64_t key, char* value) {
    page_t page;
    //node_page_t* page = (node_page_t*)malloc(PAGESIZE);

    file_read_page((leaf), &page);

    int i;
    int insertion_point = find_key_index(&page, key) + 1;

    // Shift
    for (i = ((node_page_t*)&page)->number_of_keys; i > insertion_point; i--) {
        ((node_page_t*)&page)->records[i].key = ((node_page_t*)&page)->records[i - 1].key;
        strcpy(((node_page_t*)&page)->records[i].value, ((node_page_t*)&page)->records[i - 1].value);
    }

    // Insert
    ((node_page_t*)&page)->records[insertion_point].key = key;
    strcpy(((node_page_t*)&page)->records[insertion_point].value, value);

    ((node_page_t*)&page)->number_of_keys++;

    file_write_page((leaf), &page);

    //free(page);
    return leaf;
}

/* Case: need to balance */
pagenum_t insert_into_leaf_after_splitting(pagenum_t root, pagenum_t page_number, uint64_t key, char* value) {
    page_t page;
    //node_page_t* page = (node_page_t*)malloc(PAGESIZE);
    file_read_page((page_number), &page);

    page_t new_page;
    pagenum_t new_page_number;
    new_page_number = make_page(1);

    //node_page_t* new_page = (node_page_t*)malloc(PAGESIZE);
    file_read_page((new_page_number), &new_page);

    int64_t temp_keys[LEAF_BRANCH_FACTOR];
    char temp_values[LEAF_BRANCH_FACTOR][120];

    int i, j, split, num_keys;
    int insertion_point = find_key_index(&page, key) + 1;

    // Copy to temp space
    for (i = 0, j = 0; i < ((node_page_t*)&page)->number_of_keys; i++, j++) {
        if (j == insertion_point) j++;
        temp_keys[j] = ((node_page_t*)&page)->records[i].key;
        strcpy(temp_values[j], ((node_page_t*)&page)->records[j].value);
    }

    // Insert
    temp_keys[insertion_point] = key;
    strcpy(temp_values[insertion_point], value);

    split = cut(LEAF_BRANCH_FACTOR - 1);

    // Split the records into two leafs
    for (i = 0, num_keys = 0; i < split; i++) {
        ((node_page_t*)&page)->records[i].key = temp_keys[i];
        strcpy(((node_page_t*)&page)->records[i].value, temp_values);
        ((node_page_t*)&page)->number_of_keys = ++num_keys;
    }
    
    for (i = split, j = 0, num_keys = 0; i < LEAF_BRANCH_FACTOR; i++, j++) {
        ((node_page_t*)&page)->records[j].key = temp_keys[i];
        strcpy(((node_page_t*)&page)->records[j].value, temp_values[i]);
        ((node_page_t*)&page)->number_of_keys = ++num_keys;
    }

    // Set new page (right node) a new page number
    ((node_page_t*)&new_page)->right_sibling_page_number = ((node_page_t*)&page)->right_sibling_page_number;
    ((node_page_t*)&page)->right_sibling_page_number = new_page_number;

    for (i = ((node_page_t*)&page)->number_of_keys; i < LEAF_BRANCH_FACTOR - 1; i++) {
        ((node_page_t*)&page)->records[i].key = 0;
        strcpy(((node_page_t*)&page)->records[i].value, "");
    }
    for (i = ((node_page_t*)&new_page)->number_of_keys; i < LEAF_BRANCH_FACTOR; i++) {
        ((node_page_t*)&new_page)->records[i].key = 0;
        strcpy(((node_page_t*)&new_page)->records[i].value, "");
    }

    // Set parent page number
    ((node_page_t*)&new_page)->parent_page_number = ((node_page_t*)&page)->parent_page_number;

    // Key to push up
    uint64_t up_key = find_key(&new_page, 0);

    file_write_page((page_number), &page);
    file_write_page((new_page_number), &new_page);

    //free(new_page);
    //free(page);
    return insert_into_parent(root, page_number, up_key, new_page_number);
}

/* Insert key into parent page */
pagenum_t insert_into_parent(pagenum_t root, pagenum_t left_number, uint64_t key, pagenum_t right_number) {
    int left_index = 0;
    pagenum_t parent_number;
    
    page_t left, parent;
    //page_t* left = (page_t*)malloc(PAGESIZE);
    file_read_page((left_number), &left);

    parent_number = ((node_page_t*)&left)->parent_page_number;

    //page_t* parent = (page_t*)malloc(PAGESIZE);
    file_read_page((parent_number), &parent);

    // Case: need to make new root
    if(parent_number == 0) {
        //free(left);
        //free(parent);
        return insert_into_new_root(left_number, key, right_number);
    }

    // Find the parent's pointer to left page
    while (left_index <= ((node_page_t*)&parent)->number_of_keys && ((node_page_t*)&parent)->entries[left_index - 1].page_number != left_number) {
        left_index ++;
    }

    if (((node_page_t*)&parent)->number_of_keys < INTERNAL_BRANCH_FACTOR - 1) {
        //free(left);
        //free(parent);
        return insert_into_node(root, parent_number, left_index, key, right_number);
    }

    //free(parent);
    //free(left);
    return insert_into_node_after_splitting(root, parent_number, left_index, key, right_number);
}

/* Insert into internal page */
pagenum_t insert_into_node(pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number) {
    page_t parent;
    //page_t* parent = (page_t*)malloc(PAGESIZE);

    file_read_page((parent_number), &parent);

    int i;

    for (i = ((node_page_t*)&parent)->number_of_keys; i > left_index; i--) {
        ((node_page_t*)&parent)->entries[i].key = ((node_page_t*)&parent)->entries[i - 1].key;
        ((node_page_t*)&parent)->entries[i + 1].page_number = ((node_page_t*)&parent)->entries[i].page_number;
    }

    ((node_page_t*)&parent)->entries[left_index].key = key;
    if (left_index == -1) {
        ((node_page_t*)&parent)->one_more_page_number = right_number;
    }
    else {
        ((node_page_t*)&parent)->entries[left_index].page_number = right_number;
    }

    ((node_page_t*)&parent)->number_of_keys++;

    file_write_page((parent_number), &parent);

    //free(parent);
    return root;
}

/* Insert into internal page after splitting */
pagenum_t insert_into_node_after_splitting(pagenum_t root, pagenum_t parent_number, int left_index, uint64_t key, pagenum_t right_number) {
    page_t parent, child, new_page;
    //page_t* parent = (page_t*)malloc(PAGESIZE);
    file_read_page((parent_number), &parent);
    
    pagenum_t new_page_number, child_number;
    //page_t* child = (page_t*)malloc(PAGESIZE);
    //page_t* new_page = (page_t*)malloc(PAGESIZE);
    uint64_t up_key;
    uint64_t temp_keys[INTERNAL_BRANCH_FACTOR];
    uint64_t temp_pointers[INTERNAL_BRANCH_FACTOR + 1];

    int i, j, split, num_keys;
    for (i = 0, j = 0; i < ((node_page_t*)&parent)->number_of_keys + 1; i++, j++) {
        if (j == left_index + 1) {
            j++;
        }
        temp_pointers[j] = ((node_page_t*)&parent)->entries[i].page_number;
    }

    for (i = 0, j = 0; i < ((node_page_t*)&parent)->number_of_keys; i++, j++) {
        if (j == left_index) {
            j++;
        }
        temp_keys[j] = ((node_page_t*)&parent)->entries[i].key;
    }

    temp_pointers[left_index + 1] = right_number;
    temp_keys[left_index] = key;

    split = cut(INTERNAL_BRANCH_FACTOR);
    new_page_number = make_page(0);
    file_read_page((new_page_number), &new_page);
    
    // left internal page
    for (i = 0, num_keys = 0; i < split - 1; i++) {
        ((node_page_t*)&parent)->entries[i].page_number = temp_pointers[i];
        ((node_page_t*)&parent)->entries[i].key = temp_keys[i];
        ++num_keys;
    }
    ((node_page_t*)&parent)->number_of_keys = num_keys;
    ((node_page_t*)&parent)->entries[split - 1].page_number = temp_pointers[split - 1];
    up_key = temp_keys[split - 1];

    //right internal page
    for (++i, j = 0, num_keys = 0; i < INTERNAL_BRANCH_FACTOR; i++, j++) {
        ((node_page_t*)&new_page)->entries[j].page_number = temp_pointers[i];
        ((node_page_t*)&new_page)->entries[j].key = temp_keys[i];
        ++num_keys;
    }
    ((node_page_t*)&new_page)->entries[j].page_number = temp_pointers[i];

    ((node_page_t*)&new_page)->parent_page_number = ((node_page_t*)&parent)->parent_page_number;
    for (i = 0; i <= ((node_page_t*)&new_page)->number_of_keys; i++) {
        child_number = ((node_page_t*)&new_page)->entries[i].page_number;
        file_read_page((child_number), &child);
        ((node_page_t*)&child)->parent_page_number = new_page_number;
        file_write_page((child_number), &child);
    }

    file_write_page((new_page_number), &new_page);
    file_write_page((parent_number), &parent);

    //free(new_page);
    //free(child);
    //free(parent);
    return insert_into_parent(root, parent_number, up_key, new_page_number);
}

/* Case: new root is needed */
pagenum_t insert_into_new_root(pagenum_t left_number, uint64_t key, pagenum_t right_number) {
    page_t root, left, right;
    pagenum_t root_number = make_page();
    //page_t* root = (page_t*)malloc(PAGESIZE);
    file_read_page(root_number, &root);

    //page_t* left = (page_t*)malloc(PAGESIZE);
    file_read_page((left_number), &left);

    //page_t* right = (page_t*)malloc(PAGESIZE);
    file_read_page((right_number), &right);

    ((node_page_t*)&root)->entries[0].key = key;
    ((node_page_t*)&root)->entries[0].page_number = left_number;
    ((node_page_t*)&root)->entries[1].page_number = right_number;
    ((node_page_t*)&root)->number_of_keys = 1;
    ((node_page_t*)&root)->is_leaf = 0;
    ((node_page_t*)&root)->parent_page_number = 0;
    ((node_page_t*)&left)->parent_page_number = root_number;
    ((node_page_t*)&right)->parent_page_number = root_number;

    file_write_page((root_number), &root);
    file_write_page((left_number), &left);
    file_write_page((right_number), &right);

    //free(right);
    //free(left);
    //free(root);
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
int db_delete(int64_t key) {
    //header_page_t* header = (header_page_t*)malloc(PAGESIZE);
    file_read_page(0, &header);

    pagenum_t root_number;
    root_number = delete((&header)->root_page_number, key);

    if (root_number == -1) {
        return -1;
    }

    (&header)->root_page_number = root_number;

    file_write_page(0, &header);

    //free(header);
    return 0;
}

pagenum_t delete(pagenum_t root, uint64_t key) {
    pagenum_t leaf_number = find_leaf_page(root, key);
    if (leaf_number != 0) {
        root = delete_entry(root, leaf_number, key);
    }
    return root;
}

pagenum_t delete_entry(pagenum_t root, pagenum_t leaf_number, uint64_t key) {
    leaf_number = remove_entry_from_node(leaf_number, key);

    page_t leaf;
    //page_t* leaf = (page_t*)malloc(PAGESIZE);
    file_read_page((leaf_number), &leaf);


    // Case: deletion from root
    if (leaf_number == root) {
        return adjust_root(root);
    }

    // Case: deletion from non-root
    // Delayed Merge
    if (((node_page_t*)&leaf)->number_of_keys <= 0) {
        root = coalesce_nodes(root, leaf_number);
    }

    //free(leaf);
    return root;
}

pagenum_t remove_entry_from_node(pagenum_t leaf_number, uint64_t key) {
    page_t leaf;
    //page_t* leaf = (page_t*)malloc(PAGESIZE);
    file_read_page((leaf_number), &leaf);

    int i, j;

    i = find_key_index(&leaf, key);

    // Remove record in leaf page
    for (j = i; j < ((node_page_t*)&leaf)->number_of_keys; j++) {
        ((node_page_t*)&leaf)->records[j].key = ((node_page_t*)&leaf)->records[j + 1].key;
        strcpy(((node_page_t*)&leaf)->records[j].value, ((node_page_t*)&leaf)->records[j + 1].value);
    }

    ((node_page_t*)&leaf)->number_of_keys--;
    ((node_page_t*)&leaf)->records[((node_page_t*)&leaf)->number_of_keys].key = 0;
    strcpy(((node_page_t*)&leaf)->records[((node_page_t*)&leaf)->number_of_keys].value, "");

    file_write_page((leaf_number), &leaf);

    //free(leaf);
    return leaf_number;
}

pagenum_t adjust_root(pagenum_t root_number) {
    page_t root;
    //page_t* root = (page_t*)malloc(PAGESIZE);
    file_read_page((root_number), &root);
    
    pagenum_t new_root_number;

    if (((node_page_t*)&root)->number_of_keys > 0) {
        //free(root);
        return root_number;
    }

    if (((node_page_t*)&root)->is_leaf == 0) {
        new_root_number = ((node_page_t*)&root)->one_more_page_number;
        page_t new_root;
        //page_t* new_root = (page_t*)malloc(PAGESIZE);
        file_read_page((new_root_number),&new_root);
        ((node_page_t*)&new_root)->parent_page_number = 0;
        file_write_page((new_root_number),&new_root); 
        //free(new_root);
    }

    else {
        new_root_number = 0;
    }

    file_free_page(root_number);

    return new_root_number;
}

pagenum_t get_neighbor_number(pagenum_t n) {
    pagenum_t c = n;
    page_t page;
    //page_t* page = (page_t*)malloc(PAGESIZE);
    file_read_page((c),&page);

    c = ((node_page_t*)&page)->parent_page_number;
    file_read_page((c),&page);

    int i;
    for (i = 0; i < ((node_page_t*)&page)->number_of_keys; i++) {
        if (((node_page_t*)&page)->entries[i].page_number == n) {
            if (i > 0) {
                // Case: page has left neighbor
                return ((node_page_t*)&page)->entries[i - 1].page_number;
            }
            else {
                // Case: page is left most child
                return ((node_page_t*)&page)->entries[1].page_number;
            }
        }
    }
    //free(page);
    return 0;
}

pagenum_t coalesce_nodes(pagenum_t root, pagenum_t page_number_to_free) {
    page_t free_page;
    //page_t* free_page = (page_t*)malloc(PAGESIZE);
    file_read_page((page_number_to_free), &free_page);

    int i, j;

    pagenum_t parent_number, neighbor_number, child_number;

    parent_number = ((node_page_t*)&free_page)->parent_page_number;
    
    if (parent_number == 0) {
        return adjust_root(root);
    }    

    page_t parent_page;
    //page_t* parent_page = (page_t*)malloc(PAGESIZE);
    file_read_page((parent_number), &parent_page);

    if (((node_page_t*)&free_page)->is_leaf == 1) { // Case: leaf page
        // Append free_page's entries on neighbor_page
        neighbor_number = get_neighbor_number(page_number_to_free);

        page_t neighbor_page;
        //page_t* neighbor_page = (page_t*)malloc(PAGESIZE);
        file_read_page((neighbor_number), &neighbor_page);
    
        int neighbor_insertion_index = ((node_page_t*)&neighbor_page)->number_of_keys;

        for (i = neighbor_insertion_index, j = 0; j < ((node_page_t*)&free_page)->number_of_keys; i++, j++) {
            ((node_page_t*)&neighbor_page)->records[i].key = ((node_page_t*)&free_page)->records[j].key;
            strcpy(((node_page_t*)&neighbor_page)->records[i].value, ((node_page_t*)&free_page)->records[j].value);
        }

        // Move the right sibling page number
        ((node_page_t*)&neighbor_page)->right_sibling_page_number = ((node_page_t*)&free_page)->right_sibling_page_number;

        file_write_page((neighbor_number), &neighbor_page);

        // Remove the page from parent
        for (i = 0; i < ((node_page_t*)&parent_page)->number_of_keys; i++) {
            if (page_number_to_free == ((node_page_t*)&parent_page)->entries[i].page_number) {
                break;
            }
        }
        for (j = i + 1; j < ((node_page_t*)&parent_page)->number_of_keys; j++) {
            if (i != 0) {
                ((node_page_t*)&parent_page)->entries[j - 1].key = ((node_page_t*)&parent_page)->entries[j].key;
            }
            ((node_page_t*)&parent_page)->entries[j].page_number = ((node_page_t*)&parent_page)->entries[j + 1].page_number;
        }
        // Clear last index page
        ((node_page_t*)&parent_page)->entries[j - 1].key = 0;
        ((node_page_t*)&parent_page)->entries[j].page_number = 0;
        ((node_page_t*)&parent_page)->number_of_keys--;
        //free(neighbor_page);
    }
    else { // Case: internal page
        // Get the number of only child of the page to free
        child_number = ((node_page_t*)&free_page)->one_more_page_number;

        page_t child_page;
        //page_t* child_page = (page_t*)malloc(PAGESIZE);
        file_read_page((child_number), &child_page);

        // Search index of the parent which point the page to free
        for (i = 0; i < ((node_page_t*)&parent_page)->number_of_keys; i++) {
            if (page_number_to_free == ((node_page_t*)&parent_page)->entries[i].page_number) {
                break;
            }
        }

        // Connect free_page's child to free_page's parent
        if (i != 0) {
            ((node_page_t*)&parent_page)->entries[i - 1].key = ((node_page_t*)&parent_page)->entries[i].key;
        }
        ((node_page_t*)&parent_page)->entries[i].page_number = child_number;
        ((node_page_t*)&child_page)->parent_page_number = parent_number;

        file_write_page((child_number), &child_page);
        //free(child_page);
    }

    file_write_page((parent_number), &parent_page);

    // Call coalesce_nodes() recursively when parent_page becomes empty
    if (((node_page_t*)&parent_page)->number_of_keys <= 0) {
        root = coalesce_nodes(root, parent_number);
    }

    file_free_page(page_number_to_free);

    //free(parent_page);
    //free(free_page);
    return root;
}


/*
 * Print functions
 */

 /*
void enqueue(pagenum_t new_node_number) {
    node_t* c = Q;
    node_t* new_node = (node_t*)malloc(sizeof(node_t));

    new_node->page_number = new_node_number;
    new_node->next = NULL;

    if (Q == NULL) {
        Q = new_node;
    }
    else {
        while (c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
    }
    free(new_node);
}

pagenum_t dequeue() {
    if (Q == NULL) {
        return 0;
    }

    pagenum_t page_num = Q->page_number;

    Q = Q->next;

    return page_num;
}

void print_leaves(pagenum_t root) {
    int i;
    pagenum_t c = root;
    
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    page_t page;
    //page_t* page = (page_t*)malloc(PAGESIZE);
    file_read_page((c), &page);

    while (!((node_page_t*)&page)->is_leaf) {
        c = ((node_page_t*)&page)->one_more_page_number;
        file_read_page((c), &page);
    }

    while(true) {
        for (i = 0; i < ((node_page_t*)&page)->number_of_keys; i++) {
            printf("{ %lu } %s ", find_key(&page,i), find_value(&page,i));
        }
        if (((node_page_t*)&page)->right_sibling_page_number != 0) {
            printf(" | ");
            c = ((node_page_t*)&page)->right_sibling_page_number;
            file_read_page((c), &page);
        }
        break;
    }
    printf("\n");
}

void print_tree (pagenum_t root) {
    pagenum_t c;
    page_t page;
    //page_t* page = (page_t*)malloc(PAGESIZE);
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }

    Q = NULL;
    enqueue(root);
    while (Q != NULL) {
        c = dequeue();
        file_read_page((c), &page);
        if (((node_page_t*)&page)->parent_page_number != 0) {
            new_rank = path_to_root(root, c);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        for (i = 0; i < ((node_page_t*)&page)->number_of_keys; i++) {
            printf("{ %lu } ", find_key(&page,i));
        }
        if (!((node_page_t*)&page)->is_leaf) {
            for (i = 0; i < ((node_page_t*)&page)->number_of_keys; i++) {
                enqueue(((node_page_t*)&page)->entries[i].page_number);
            }
        }
        printf("| ");
    }
    printf("\n");
    
}

int path_to_root(pagenum_t root, pagenum_t page_number) {
    pagenum_t c = page_number;
    int length = 0;
    page_t page;
    //page_t* page = (page_t*)malloc(PAGESIZE);
    file_read_page((c), &page);
    while (c != root) {
        c = ((node_page_t*)&page)->parent_page_number;
        length++;
    }
    return length;
}

*/