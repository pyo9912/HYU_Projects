// 2019094511 ±Ë¡ÿ«• 12838
#define _CRT_SECURE_NO_WARNINGS
#define INF 10000000000
#include <stdio.h>

int** matrix(int* p, int n, int** s) { 
	int** m = new int* [n + 1];
	for (int i = 0; i <= n; i++) {
		m[i] = new int[n + 1];
	}
	for (int i = 1; i <= n; i++) {
		m[i][i] = 0;	
	}
	int j;
	for (int l = 1; l <= n - 1; l++) {
		for (int i = 1; i <= n - l; i++) {
			j = i + l;
			m[i][j] = INF;

			for (int k = i; k < j; k++) {
				if (m[i][j] > m[i][k] + m[k + 1][j] + p[i - 1] * p[k] * p[j]) {
					m[i][j] = m[i][k] + m[k + 1][j] + p[i - 1] * p[k] * p[j];
					s[i][j] = k;
				}
			}
		}
	}
	return m;
}

void PrintMultorder(int** s, int i, int j) {
	if (i == j) {
		printf("%d", i);
		return;
	}
	int k = s[i][j];
	printf("(");
	PrintMultorder(s, i, k);
	PrintMultorder(s, k + 1, j);
	printf(")");
}

int main() {
	int n, i;
	scanf("%d", &n);
	int* p = new int[n + 1];
	for (i = 0; i < n + 1; i++) {
		scanf("%d", &p[i]);
	}
	int** s = new int* [n + 1];
	for (i = 0; i <= n; i++) {
		s[i] = new int[n + 1];
	}
	int** m = matrix(p, n, s);
	printf("%d\n", m[1][n]);
	PrintMultorder(s, 1, n);
	return 0;
}