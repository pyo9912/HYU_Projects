// 2019094511_KimJunpyo_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stack>

using namespace std;

int N, M;
stack<int> Stack;
bool adj[1000][1000];
bool finish[1000];
bool visit[1000];
bool DAG = true;

void DFS(int start)
{
	if (finish[start]) {
		DAG = false;
		return;
	}
	finish[start] = true;
	for (int i = 1; i <= N; i++) {
		if (adj[start][i] && !visit[i]) {
			DFS(i);
		}
	}
	visit[start] = true;
	Stack.push(start);

	return;
}

int main()
{
	scanf("%d %d", &N, &M);
	int u, v;
	for (int i = 0; i < M; i++) {
		scanf("%d %d", &u, &v);
		adj[u][v] = true;
	}
	for (int i = 1; i <= N; i++) {
		if (!visit[i]) {
			DFS(i);
		}
	}
	if (Stack.size() != N) {
		DAG = false;
	}
	printf("%d\n", DAG);
	if (DAG) {
		while (!Stack.empty()) {
			printf("%d ", Stack.top());
			Stack.pop();
		}
		printf("\n");
	}
	return 0;
}