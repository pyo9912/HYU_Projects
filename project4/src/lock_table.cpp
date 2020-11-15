#include <lock_table.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <unordered_map>
using namespace std;

class pairHash {
	public:
		size_t operator()(const pair<int,int64_t>&k) const {
			return k.first*1000+k.second;
		}
};

unordered_map <pair<int,int64_t>,lock_entry_t*, pairHash> table;

pthread_mutex_t lock_table_latch;

int
init_lock_table()
{
	lock_table_latch = PTHREAD_MUTEX_INITIALIZER;

	return 0;
}

lock_t*
lock_acquire(int table_id, int64_t key)
{
	pthread_mutex_lock(&lock_table_latch);
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));

	auto iter = table.find(make_pair(table_id, key));

	if (iter == table.end()) {
		// case: no lock entry
		lock_entry_t* lock_entry = (lock_entry_t*)malloc(sizeof(lock_entry_t));
		lock_entry->table_id = table_id;
		lock_entry->record_id = key;
		lock_entry->tail = lock_entry->head = lock;
		lock->mode = ACQUIRED;
		lock->cond = PTHREAD_COND_INITIALIZER;
		lock->sentinel = lock_entry;
		lock->prev = lock->next = NULL;
		table.insert(make_pair(make_pair(table_id, key), lock_entry));
		
		pthread_mutex_unlock(&lock_table_latch);
		return lock;
	}
	else {
		// case: lock entry exist
		lock_entry_t* lock_entry = iter->second;
		lock_t* c = lock_entry->tail;
		if (c == NULL) {
			// case: no lock in lock entry
			lock->mode = ACQUIRED;
			lock->cond = PTHREAD_COND_INITIALIZER;
			lock->sentinel = lock_entry;
			lock_entry->head = lock;
			lock_entry->tail = lock;
			lock->prev = NULL;
			lock->next = NULL;

			pthread_mutex_unlock(&lock_table_latch);
			return lock;
		}
		else {
			// case: lock exist in lock entry
			lock->mode = WAITING;
			lock->cond = PTHREAD_COND_INITIALIZER;
			lock->sentinel = lock_entry;
			c->next = lock;
			lock->next = NULL;
			lock->prev = c;
			lock_entry->tail = lock;

			//pthread_cond_signal(&cond);
			while (lock->mode == WAITING) {
				pthread_cond_wait(&lock->cond, &lock_table_latch);
			}
			pthread_mutex_unlock(&lock_table_latch);
			return lock;
		}
	}
	pthread_mutex_unlock(&lock_table_latch);

	return nullptr;
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&lock_table_latch);
	lock_entry_t* lock_entry = lock_obj->sentinel;
	if (lock_obj->prev == NULL) {
		// case: head point to lock_obj
		if (lock_obj->next != NULL) {
			lock_obj->next->prev = NULL;
			lock_obj->next->mode = ACQUIRED;
			lock_entry->head = lock_obj->next;

			pthread_cond_signal(&lock_obj->next->cond);
			free(lock_obj);
		}
		else {
			lock_entry->head = NULL;
			lock_entry->tail = NULL;
			free(lock_obj);
		}
	}
	else {
		// case: head do not point to lock_obj
		// No need to consider yet
	}
	pthread_mutex_unlock(&lock_table_latch);

	return 0;
}
