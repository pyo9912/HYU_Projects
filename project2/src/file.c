#include "bpt.h"
#include "file.h"

int fd;

header_page_t header;

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest) {
	int nr;
	nr = pread(fd, dest, PAGESIZE, pagenum * PAGESIZE);
	if (nr == -1) {
		fprintf(stderr, "READ ERROR %d: %s\n", errno, strerror(errno));
		perror("Failed to read page");
	}

}

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src) {
	int nr;
	nr = pwrite(fd, src, PAGESIZE, pagenum * PAGESIZE);
	if (nr == -1) {
		fprintf(stderr, "WRITE ERROR %d: %s\n", errno, strerror(errno));
		perror("Failed to write page");
	}
	
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page() {
	// header_page_t* header = (header_page_t*)malloc(PAGESIZE);
	uint64_t go_to_free_page_number;

	// Allocate new page 
	page_t new_page;
	//page_t* new_page = (page_t*)malloc(PAGESIZE);
	memset(&new_page, 0, PAGESIZE);

	// Get the free page number (offset)
	file_read_page(STARTNUM, ((page_t*)&header));
	go_to_free_page_number = (&header)->free_page_number;
	if (go_to_free_page_number==0) {
		/* Case: no free pages yet */
		go_to_free_page_number = ((&header)->number_of_pages);
		(&header)->free_page_number = go_to_free_page_number;
		(&header)->number_of_pages++;

		file_write_page(0, ((page_t*)&header));
	}
	else {
		/* Case: free page is already exist */
		page_t temp;
		//page_t* temp = (page_t*)malloc(PAGESIZE);
		uint64_t new_free_page_number;

		file_read_page(go_to_free_page_number, &temp);

		new_free_page_number = ((free_page_t*)&temp)->next_free_page_number;
		(&header)->free_page_number = new_free_page_number;
		(&header)->number_of_pages++;
		//free(temp);

		file_write_page(0, ((page_t*)&header));
	}

	// Write new page on disk
	file_write_page(go_to_free_page_number, &new_page);
	//free(new_page);

	// Update header
	file_write_page(0, ((page_t*)&header));
	//free(header);

	return go_to_free_page_number;
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum) {
	// header_page_t* header = (header_page_t*)malloc(PAGESIZE);
	file_read_page(0, ((page_t*)&header));

	page_t temp;
	//page_t* temp = (page_t*)malloc(PAGESIZE);
	file_read_page(pagenum, &temp);

	// Append the page to the free page list
	((free_page_t*)&temp)->next_free_page_number = (&header)->free_page_number;
	(&header)->free_page_number = pagenum;

	// Apply changes
	file_write_page(pagenum, &temp);
	//free(temp);

	// Update header
	file_write_page(0, ((page_t*)&header));
	//free(header);
}
