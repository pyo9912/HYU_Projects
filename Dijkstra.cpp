// 2019094511_KimJunpyo_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#define INF 999999

using namespace std;

int main()
{
	int N, M;
	int x, y, w;
	scanf("%d %d", &N, &M);
	vector<pair<int,int> >* arr = new vector<pair<int, int> >[N + 1];
	for (int i = 0; i < M; i++) {
		scanf("%d %d %d", &x, &y, &w);
		arr[x].push_back({ y,w });
	}
	int* dist = new int[N + 1];
	for (int i = 0; i <= N; i++) {
		dist[i] = INF;
	}
	priority_queue<pair<int, int> > pq;

	pq.push({ 0,1 });
	dist[1] = 0;

	while (!pq.empty()) {
		int cost = -pq.top().first;
		int here = pq.top().second;
		pq.pop();

		for (int i = 0; i < arr[here].size(); i++) {
			int next = arr[here][i].first;
			int nextcost = arr[here][i].second;

			if (dist[next] > dist[here] + nextcost) {
				dist[next] = dist[here] + nextcost;
				pq.push({ -dist[next],next });
			}
		}
	}
	int res = -INF;
	for (int i = 1; i <= N; i++) {
		if (res < dist[i]) res = dist[i];
	}
	printf("%d", res);

	return 0;
}