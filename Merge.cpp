//2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

void merge(int* arr, int* sort, int p, int q, int r) {
	int i = p;
	int j = q + 1;
	int k = p;
	while (i <= q && j <= r) {
		if (arr[i] <= arr[j]) sort[k++] = arr[i++];
		else sort[k++] = arr[j++];
	}
	while (i <= q) sort[k++] = arr[i++];
	while (j <= r) sort[k++] = arr[j++];
	for (i = p; i <= r; i++) {
		arr[i] = sort[i];
	}
}

void merge_sort(int* arr, int* sort, int p, int r) {
	if (p < r) {
		int q = (p + r) / 2;
		merge_sort(arr, sort, p, q);
		merge_sort(arr, sort, q + 1, r);
		merge(arr, sort, p, q, r);
	}
}

int main() {
	int N;
	scanf("%d", &N);
	int* arr = new int[N];
	int* sort = new int[N];
	for (int i = 0; i < N; i++) {
		scanf("%d", &arr[i]);
	}
	merge_sort(arr, sort, 0, N - 1);
	for (int i = N - 1; i >= 0; i--) {
		printf("%d\n", arr[i]);
	}
	return 0;
}