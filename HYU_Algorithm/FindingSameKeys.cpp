//2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#define MAXSIZE 100000
#include <stdio.h>

int A[MAXSIZE + 1];
int B[MAXSIZE + 1];

void init() {
	for (int i = 0; i <= MAXSIZE; i++) {
		A[i] = 0;
		B[i] = 0;
	}
}

int count(int* A, int* B) {
	int cnt = 0;
	for (int i = 0; i <= MAXSIZE; i++) {
		if (A[i] != 0 && B[i] != 0) cnt++;
	}
	return cnt;
}

int main() {
	int N, M;
	int key;
	scanf("%d %d", &N, &M);
	for (int i = 0; i < N; i++) {
		scanf("%d", &key);
		A[key]++;
	}
	for (int i = 0; i < M; i++) {
		scanf("%d", &key);
		B[key]++;
	}
	printf("%d", count(A, B));

	return 0;
}