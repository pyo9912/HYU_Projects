#include "bpt.h"
#include "file.h"
#include "buffer.h"
#include "trx.h"
#include "type.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
using namespace std;
// MAIN

// header_page_t header;

int table_id = 1;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void* find(void* arg){
	int trx_id = trx_begin();
	int64_t key;
	int success;
	char val[120];
    key = 10;
	success = db_find(table_id, key, val, trx_id);
	printf("DB_FIND ID: %d SEARCH: %d - %s\n",trx_id,key,val);
    key = 20;
	success = db_find(table_id, key, val, trx_id);
	printf("DB_FIND ID: %d SEARCH: %d - %s\n",trx_id,key,val);
	key = 30;
	success = db_find(table_id, key, val, trx_id);
	printf("DB_FIND ID: %d SEARCH: %d - %s\n",trx_id,key,val);
	key = 40;
	success = db_find(table_id, key, val, trx_id);
	printf("DB_FIND ID: %d SEARCH: %d - %s\n",trx_id,key,val);
	int result = trx_commit(trx_id);
}

void* update(void* arg) {
    int trx_id = trx_begin();
    uint64_t key;
    int success;
    key = 10;
	success = db_update(table_id, key, const_cast<char*>("aaaa"), trx_id);
    printf("DB_UPDATE ID: %d UPDATE: %d - %s\n",trx_id,key,"aaaa");
	key = 20;
	success = db_update(table_id, key, const_cast<char*>("bbbb"), trx_id);
    printf("DB_UPDATE ID: %d UPDATE: %d - %s\n",trx_id,key,"bbbb");
	key = 30;
	success = db_update(table_id, key, const_cast<char*>("cccc"), trx_id);
    printf("DB_UPDATE ID: %d UPDATE: %d - %s\n",trx_id,key,"cccc");
	key = 40;
	success = db_update(table_id, key, const_cast<char*>("dddd"), trx_id);
    printf("DB_UPDATE ID: %d UPDATE: %d - %s\n",trx_id,key,"dddd");
	int result = trx_commit(trx_id);
}

int main(){
	init_db(10);
	open_table(const_cast<char*>("test.db"));
    db_insert(table_id, 10, "xxxx");
    db_insert(table_id, 20, "xxxx");
    db_insert(table_id, 30, "xxxx");
    db_insert(table_id, 40, "xxxx");
    printf("---------------------\n");
	for(int i = 0; i < 8; i++){
		pthread_t tx1, tx2;
		pthread_create(&tx1, 0, find, NULL);
		pthread_create(&tx2, 0, update, NULL);
	}	
	shutdown_db();

    return 0;
}