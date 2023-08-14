// 2019094511 ±Ë¡ÿ«• 12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int main() {
	int N, i, j, max;
	scanf("%d", &N);
	int* p = new int[N + 1];
	int* r = new int[N + 1];
	int* s = new int[N + 1];
	p[0] = 0;
	r[0] = 0;
	s[0] = 0;

	for (i = 1; i <= N; i++) {
		scanf("%d", &p[i]);

		max = p[i];
		s[i] = i;
		for (j = 1; j < i; j++) {
			if (max < p[j] + r[i - j]) {
				max = p[j] + r[i - j];
				s[i] = j;
			}
		}
		r[i] = max;
	}
	printf("%d\n", r[N]);
	int n = N;
	while (n > 0) {
		printf("%d ", s[n]);
		n = n - s[n];
	}
	return 0;
}