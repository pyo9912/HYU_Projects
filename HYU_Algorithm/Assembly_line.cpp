// 2019094511 ±Ë¡ÿ«• 12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int main() {
	int N, e1, e2, x1, x2;
	scanf("%d %d %d %d %d", &N, &e1, &e2, &x1, &x2);
	int i, j;
	int **s, ** a, ** t, ** l;
	int* rec = new int[N];
	s = new int* [2];
	a = new int* [2];
	t = new int* [2];
	l = new int* [2];

	for (i = 0; i < 2; i++) {
		s[i] = new int[N];
		a[i] = new int[N];
		t[i] = new int[N];
		l[i] = new int[N];
	}

	for (i = 0; i < 2; i++) {
		for (j = 0; j < N; j++) {
			scanf("%d", &a[i][j]);
		}
	}
	for (i = 0; i < 2; i++) {
		for (j = 0; j < N - 1; j++) {
			scanf("%d", &t[i][j]);
		}
	}

	s[0][0] = e1 + a[0][0];
	s[1][0] = e2 + a[1][0];
	l[0][0] = 0;
	l[0][1] = 1;

	for (j = 1; j < N; j++) {
		if (s[0][j - 1] <= s[1][j - 1] + t[1][j - 1]) {
			s[0][j] = s[0][j - 1] + a[0][j];
			l[0][j] = 0;
		}
		else {
			s[0][j] = s[1][j - 1] + t[1][j - 1] + a[0][j];
			l[0][j] = 1;
		}
		if (s[1][j - 1] <= s[0][j - 1] + t[0][j - 1]) {
			s[1][j] = s[1][j - 1] + a[1][j];
			l[1][j] = 1;
		}
		else {
			s[1][j] = s[0][j - 1] + t[0][j - 1] + a[1][j];
			l[1][j] = 0;
		}
	}
	int line = 0;
	if (s[0][N - 1] + x1 <= s[1][N - 1] + x2) {
		printf("%d\n", s[0][N - 1] + x1);
		for (i = N - 1, j = 0; i >= 0; i--) {
			rec[i] = j;
			j = l[j][i];
		}
	}
	else {
		printf("%d\n", s[1][N - 1] + x2);
		for (i = N - 1, j = 1; i >= 0; i--) {
			rec[i] = j;
			j = l[j][i];
		}
	}
	for (i = 0; i < N; i++) {
		printf("%d %d\n", rec[i] + 1, i + 1);
	}
	return 0;

}