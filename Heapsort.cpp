//2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#define swap(a,b) {int t; t=a; a=b; b=t;}
#include <stdio.h>

void makeHeap(int* heap, int idx, int N) {
	int left = idx * 2 + 1;
	int right = idx * 2 + 2;
	int parent = idx;
	if (left<N && heap[left]>heap[parent]) parent = left;
	if (right<N && heap[right]>heap[parent]) parent = right;
	if (parent != idx) {
		swap(heap[idx], heap[parent]);
		makeHeap(heap, parent, N);
	}
}

void buildHeap(int* heap, int N) {
	for (int i = N / 2 - 1; i >= 0; i--) {
		makeHeap(heap, i, N);
	}
}

void HeapSort(int* heap, int N, int k) {
	int size;
	buildHeap(heap, N);
	for (size = N - 1; size >= N - k; size--) {
		swap(heap[0], heap[size]);
		makeHeap(heap, 0, size);
	}
}

int main(void) {
	int N, k;
	scanf("%d %d", &N, &k);
	int* heap = new int[N];
	for (int i = 0; i < N; i++) {
		scanf("%d", &heap[i]);
	}
	HeapSort(heap, N, k);
	for (int i = N - 1; i >= N - k; i--) {
		printf("%d ", heap[i]);
	}
	printf("\n");
	for (int i = 0; i < N- k; i++) {
		printf("%d ", heap[i]);
	}
}