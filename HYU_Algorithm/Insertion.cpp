//2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main() {
	int N;
	int i, j;
	scanf("%d", &N);
	int* arr = new int[N];
	for (i = 0; i < N; i++) {
		scanf("%d", &arr[i]);
	}
	for (j = 1; j < N; j++) {
		int key = arr[j];
		i = j - 1;
		while (i >= 0 && arr[i] > key) {
			arr[i + 1] = arr[i];
			i--;
		}
		arr[i + 1] = key;
	}
	for (i = N-1; i >= 0; i--) {
		printf("%d\n", arr[i]);
	}
	return 0;
}