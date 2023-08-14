#include "trx.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <unordered_map>
#include <list>
using namespace std;

pthread_mutex_t transaction_manager_latch;
pthread_mutex_t lock_manager_latch;

class pairHash {
	public:
		size_t operator()(const pair<int,int64_t>&k) const {
			return k.first*1000+k.second;
		}
};

unordered_map <pair<int,int64_t>,lock_entry_t*, pairHash> lock_table;
unordered_map <int,trx_t*> trx_table;

int global_transaction_id = 0;

int init_lock_table()
{
	lock_manager_latch = PTHREAD_MUTEX_INITIALIZER;
	transaction_manager_latch = PTHREAD_MUTEX_INITIALIZER;
	return 0;
}

int trx_begin() {
	pthread_mutex_lock(&transaction_manager_latch);
	global_transaction_id++;
	int tid = global_transaction_id;
	trx_t* trx = (trx_t*)malloc(sizeof(trx_t));
	if (trx == NULL) {
		pthread_mutex_unlock(&transaction_manager_latch);
		return 0;
	}
	trx->trx_id = tid;
	trx->trx_head = trx->trx_tail = NULL;
	trx_table.insert(make_pair(tid, trx));

	pthread_mutex_unlock(&transaction_manager_latch);

	return tid;
}

int trx_commit(int trx_id) {
	pthread_mutex_lock(&transaction_manager_latch);
	int tid = trx_id;
	auto trx_iter = trx_table.find(tid);
	if (trx_iter == trx_table.end()) {
		// case : No matching trx
		pthread_mutex_unlock(&transaction_manager_latch);
		return 0;
	}
	else {
		trx_t* trx = trx_iter->second;
		trx_release(trx);
		free(trx);
		pthread_mutex_unlock(&transaction_manager_latch);
		return tid;
	}

	pthread_mutex_unlock(&transaction_manager_latch);
	return trx_id;
}

void trx_release(trx_t* trx) {
	lock_t* cur = trx->trx_head;
	lock_t* next = NULL;
	while (cur != NULL) {
		next = cur->trx_next;
		lock_release(cur);
		cur = next;
	}
}

lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode)
{
	pthread_mutex_lock(&lock_manager_latch);
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
	
	if (trx_table.find(trx_id) == trx_table.end()) {
		trx_t* trx_temp = (trx_t*)malloc(sizeof(trx_t));
		trx_temp->trx_id = trx_id;
		trx_temp->trx_head = NULL;
		trx_temp->trx_tail = NULL;
		trx_table.insert(make_pair(trx_id, trx_temp));
	}
	auto trx_iter = trx_table.find(trx_id);
	auto lock_iter = lock_table.find(make_pair(table_id, key));
	if (lock_iter == lock_table.end()) {
		// case: no lock entry
		lock_entry_t* lock_entry = (lock_entry_t*)malloc(sizeof(lock_entry_t));
		lock_entry->table_id = table_id;
		lock_entry->record_id = key;
		lock_entry->tail = lock_entry->head = lock;
		lock->state = ACQUIRED;
		lock->cond = PTHREAD_COND_INITIALIZER;
		lock->sentinel = lock_entry;
		lock->prev = lock->next = NULL;
		lock->trx_id = trx_id;
		lock->is_updated = 0;
		lock->trx_next = NULL;
		if (lock_mode == 0) {
			lock->mode = SHARED;
		}
		else {
			lock->mode = EXCLUSIVE;
		}
		trx_iter->second->trx_tail = lock;
		lock_table.insert(make_pair(make_pair(table_id, key), lock_entry));
		pthread_mutex_unlock(&lock_manager_latch);
		return lock;
	}
	else {
		// case: lock entry exist
		lock_entry_t* lock_entry = lock_iter->second;
		lock_t* c = lock_entry->tail;
		if (c == NULL) {
			// case: no lock in lock entry
			lock->state = ACQUIRED;
			lock->cond = PTHREAD_COND_INITIALIZER;
			lock->sentinel = lock_entry;
			lock_entry->head = lock;
			lock_entry->tail = lock;
			lock->prev = NULL;
			lock->next = NULL;
			lock->trx_id = trx_id;
			lock->is_updated = 0;
			lock->trx_next = NULL;
			if (lock_mode == 0) {
				lock->mode = SHARED;
			}
			else {
				lock->mode = EXCLUSIVE;
			}
			trx_iter->second->trx_head = lock;
			trx_iter->second->trx_tail = lock;
			pthread_mutex_unlock(&lock_manager_latch);
			return lock;
		}
		else {
			// case: lock exist in lock entry
			// deadlock detection
			lock_t* cur = trx_iter->second->trx_head;
			while (cur != NULL) {
				lock_t* temp = lock_entry->head;
				lock_t* comp = cur;
				while (temp != NULL) {
					if (temp->trx_id == comp->trx_id) {
						// deadlock detected
						// // Abort

						pthread_mutex_unlock(&lock_manager_latch);
						return nullptr;
					}
					temp = temp->next;
				}
				cur = cur->trx_next;
			}
			// append
			lock->cond = PTHREAD_COND_INITIALIZER;
			lock->sentinel = lock_entry;
			c->next = lock;
			lock->next = NULL;
			lock->prev = c;
			lock_entry->tail = lock;
			lock->trx_id = trx_id;
			lock->is_updated = 0;
			lock->trx_next = NULL;
			if (c->mode == SHARED && c->state == ACQUIRED && lock_mode == 0) {
				lock->state = ACQUIRED;
			}
			else {
				lock->state = WAITING;
			}
			if (lock_mode == 0) {
				lock->mode = SHARED;
			}
			else {
				lock->mode = EXCLUSIVE;
			}
			if (trx_iter->second->trx_tail == NULL) {
				trx_iter->second->trx_head = lock;
				trx_iter->second->trx_tail = lock;
			}
			else {
				trx_iter->second->trx_tail->trx_next = lock;
			}
			trx_iter->second->trx_tail = lock;
			while (lock->state == WAITING) {
				pthread_cond_wait(&lock->cond, &lock_manager_latch);
			}
			pthread_mutex_unlock(&lock_manager_latch);
			return lock;
		}
	}
	pthread_mutex_unlock(&lock_manager_latch);

	return nullptr;
}

int lock_release(lock_t* lock_obj) {
	pthread_mutex_lock(&lock_manager_latch);
	lock_entry_t* lock_entry = lock_obj->sentinel;
	if (lock_obj->prev == NULL) {
		// case: head point to lock_obj
		if (lock_obj->next != NULL) {
			lock_obj->next->prev = NULL;
			lock_entry->head = lock_obj->next;
			if (lock_obj->next->mode == SHARED) {
				lock_t* c = lock_obj->next;
				while (c->mode != EXCLUSIVE) {
					c->state = ACQUIRED;
					pthread_cond_signal(&c->cond);
					c = c->next;
				}
			}
			else {
				lock_obj->next->state = ACQUIRED;
				pthread_cond_signal(&lock_obj->next->cond);
			}
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
		if (lock_obj->next != NULL) {
			if (lock_obj->mode == EXCLUSIVE && 
			lock_obj->prev->state == ACQUIRED && lock_obj->prev->mode == SHARED) {
				lock_t* c = lock_obj->next;
				while (c->mode != EXCLUSIVE) {
					c->state = ACQUIRED;
					pthread_cond_signal(&c->cond);
					c = c->next;
				}
			}
			lock_obj->prev->next = lock_obj->next;
			lock_obj->next->prev = lock_obj->prev;
			free(lock_obj);
		}
		else {
			lock_entry->tail = lock_obj->prev;
			lock_obj->prev->next = NULL;
			free(lock_obj);
		}
	}
	pthread_mutex_unlock(&lock_manager_latch);

	return 0;
}

int lock_release_all(int trx_id) {
	auto trx = trx_table.find(trx_id);
	trx_release(trx->second);
	return 0;
}

void lock_remove_trx(int trx_id) {
	trx_table.erase(trx_id);
}

trx_t* get_trx_block(int trx_id) {
	auto trx = trx_table.find(trx_id);
	trx_t* ret = trx->second;
	return ret;
}