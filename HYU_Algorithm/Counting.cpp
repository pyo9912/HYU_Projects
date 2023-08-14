//2019094511_±èÁØÇ¥_12838
#define _CRT_SECURE_NO_WARNINGS
#define MAXSIZE 100000
//#define MAX(a,b) (((a)>=(b))?(a):(b))
//#define MIN(a,b) (((a)<(b))?(a):(b))
#include <stdio.h>

int KEYS[MAXSIZE + 1];

void init() {
	for (int i = 0; i <= MAXSIZE; i++) {
		KEYS[i] = 0;
	}
}

int count(int a, int b) {
	int cnt = 0;
	//int max = MAX(a, b);
	//int min = MIN(a, b);
	for (int i = a; i <= b; i++) {
		cnt += KEYS[i];
	}
	
	return cnt;
}

int main() {
	int N, M, K;
	int key;
	init();
	scanf("%d %d %d", &N, &M, &K);
	int* A = new int[K + 1];
	int* B = new int[K + 1];
	for (int i = 1; i <=  K; i++) {
		scanf("%d %d", &A[i], &B[i]);
	}
	for (int i = 1; i <= N; i++) {
		scanf("%d", &key);
		if (key <= M) {
			KEYS[key]++;
		}
		
	}
	for (int i = 1; i <= K; i++) {
		printf("%d\n", count(A[i], B[i]));
	}
}