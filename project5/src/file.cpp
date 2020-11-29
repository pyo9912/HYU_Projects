#include "file.h"
#include "buffer.h"
#include "bpt.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int tables[TABLE_SIZE];

// open table at file manager
int file_open_table(char* pathname) {
	int table_id = 1;
    while (tables[table_id-1] != 0) {
        table_id++;
    }
    if (table_id > TABLE_SIZE) {
        perror("Table list is full");
    }

	int table_fd = open(pathname, O_CREAT | O_RDWR | O_SYNC, 0777);
	tables[table_id - 1] = table_fd;
	// printf("tablefd: %d\n",table_fd);
	// printf("tableid: %d\n",table_id);
	// printf("tables: %d\n",tables[table_id - 1]);
	if (tables[table_id-1] == -1) {
		perror("Failed to load page");
	}

    page_t header;
	if (&header == NULL) {
		perror("Failed to allocate header");
	}
    file_read_page(table_id, HEADER_NUM, &header);
    memset(&header, 0, PAGESIZE);
    ((header_page_t*)&header)->number_of_pages = 1;
    file_write_page(table_id, HEADER_NUM, &header);

	return table_id;
}

// close table at file manager
int file_close_table(int table_id) {
    close_buffer_table(table_id);
	close(tables[table_id-1]);
	tables[table_id-1] = 0;
	
    return 0;
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest) {
	if (table_id < 1 || table_id > 10) {
		perror("Invalid table id / file_read");
	}
	if (tables[table_id - 1] == 0) {
		perror("Failed to open table / file_read");
	}
	int nr;
	nr = pread(tables[table_id - 1], dest, PAGESIZE, (pagenum * PAGESIZE));
	if (nr == -1) {
		perror("Failed to read page");
	}
	else {
		//printf("pread success\n");
	}
	
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src) {
	if (table_id < 1 || table_id > 10) {
		perror("Invalid table id / file_write");
	}
	if (tables[table_id - 1] == 0) {
		perror("Failed to open table / file_write");
	}

	int nr;
	nr = pwrite(tables[table_id - 1], src, PAGESIZE, (pagenum * PAGESIZE));
	if (nr == -1) {
		perror("Failed to write page");
	}
	else {
		//printf("pwrite success\n");
	}
	
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int table_id) {
	if (table_id < 1 || table_id > 10) {
		perror("Invalid table id / file_alloc");
	}
	if (tables[table_id - 1] == 0) {
		perror("Failed to open table / file_alloc");
	}
	pagenum_t go_to_free_page_number = 0;

	// Allocate new page 
	page_t* new_page = (page_t*)malloc(PAGESIZE);
	//page_t new_page;
	memset(new_page, 0, PAGESIZE);

	// Get free page number (offset)
	page_t* header = (page_t*)malloc(PAGESIZE);
	file_read_page(table_id, HEADER_NUM, header);
	go_to_free_page_number = ((header_page_t*)header)->free_page_number;

	if (go_to_free_page_number==0) {
		/* Case: no free pages yet */
		go_to_free_page_number = (((header_page_t*)header)->number_of_pages);
		((header_page_t*)header)->number_of_pages++;
	}
	else {
		/* Case: free page is already exist */
		page_t* temp = (page_t*)malloc(PAGESIZE);
		pagenum_t new_free_page_number = 0;

		file_read_page(table_id, go_to_free_page_number, temp);
		new_free_page_number = ((free_page_t*)temp)->next_free_page_number;
		((header_page_t*)header)->free_page_number = new_free_page_number;
		free(temp);
	}

	// Write new page on disk
	file_write_page(table_id, go_to_free_page_number, new_page);

	// Update header
	file_write_page(table_id, HEADER_NUM, header);

	free(new_page);
	free(header);

	return go_to_free_page_number;
}

// Free an on-disk page to the free page list
void file_free_page(int table_id, pagenum_t pagenum) {
	if (table_id < 1 || table_id > 10) {
		perror("Invalid table id");
	}
	if (tables[table_id - 1] == 0) {
		perror("Failed to open table");
	}

	page_t header;
	file_read_page(table_id, 0, ((page_t*)&header));

	page_t temp;
	file_read_page(table_id, pagenum, &temp);

	// Append the page to the free page list
	((free_page_t*)&temp)->next_free_page_number = ((header_page_t*)&header)->free_page_number;
	((header_page_t*)&header)->free_page_number = pagenum;

	// Apply changes
	file_write_page(table_id, pagenum, &temp);

	// Update header
	file_write_page(table_id, HEADER_NUM, ((page_t*)&header));
}
