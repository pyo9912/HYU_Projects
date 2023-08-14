//2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#define swap(a,b) {int t = a; a = b; b = t;}
#define MAXSIZE 100000
#include <stdio.h>

int q[MAXSIZE + 1];
int size;

void heapifyUp(int idx) {
	while (idx != 1 && q[idx] > q[idx / 2]) {
		swap(q[idx], q[idx / 2]);
		idx /= 2;
	}
}

void heapifyDown(int idx) {
	int left = idx * 2;
	int right = idx * 2 + 1;
	int max = idx;
	if (left <= size && q[max] < q[left]) max = left;
	if (right <= size && q[max] <= q[right]) max = right;
	if (idx != max) {
		swap(q[idx], q[max]);
		heapifyDown(max);
	}
}

void push(int key) {
	int idx = ++size;
	q[idx] = key;
	heapifyUp(idx);
}

int pop() {
	int pop = q[1];
	swap(q[1], q[size]);
	size--;
	heapifyDown(1);
	return pop;
}

void modify(int idx, int key) {
	if (q[idx] < key) {
		q[idx] = key;
		heapifyUp(idx);
	}
	else {
		q[idx] = key;
		heapifyDown(idx);
	}
}

int main(void) {
	size = 0;
	while (1) {
		int C;
		scanf("%d", &C);
		if (C == 0) {
			//print & terminate
			printf("\n");
			for (int i = 1; i <= size; i++) {
				printf("%d ", q[i]);
			}
			printf("\n");
			break;
		}
		else if (C == 1) {
			//insert
			int k;
			scanf("%d", &k);
			push(k);
		}
		else if (C == 2) {
			//extract max & struct again
			printf("%d ", pop());
		}
		else if (C == 3) {
			//modificate & struct again
			int idx, key;
			scanf("%d %d", &idx, &key);
			modify(idx, key);
		}
	}

	return 0;
}