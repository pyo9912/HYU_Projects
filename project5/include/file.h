#ifndef __FILE_H__
#define __FILE_H__
#include "type.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/*  file_API */

int file_open_table(char* pathname);

int file_close_table(int table_id);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int table_id);

// Free an on-disk page to the free page list
void file_free_page(int table_id, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src);

#endif /*__FILE_H__*/