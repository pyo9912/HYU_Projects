// 2019094511_KimJunpyo_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <list>
#include <algorithm>

using namespace std;

bool visit[1000];
int conn[1000];
list<int> ls[1000];

int N, M;
int cnt;

void DFS(int v)
{
	visit[v] = true;
	conn[v] = cnt;
	for (int i = 1; i <= N; i++) {
		if (find(ls[v].begin(), ls[v].end(), i) != ls[v].end() && !visit[i]) {
			DFS(i);
		}
	}
}

int main()
{
	scanf("%d %d", &N, &M);
	int x, y;
	for (int i = 0; i < M; i++) {
		scanf("%d %d", &x, &y);
		ls[x].push_back(y);
		ls[y].push_back(x);
	}
	for (int i = 1; i <= N; i++) {
		if (!visit[i]) {
			cnt++;
			DFS(i);
		}
	}
	printf("%d\n", cnt);
}