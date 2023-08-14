// 2019094511_KimJunpyo_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;

int N, M;

typedef struct Edge {
	int u, v, weight;
}Edge;

typedef struct Vertax {
	int parent;
	int rank;
}Vertax;

vector<Edge> edge;
Vertax vertax[1000];

bool compare(const Edge& e1, const Edge& e2)
{
	if (e1.weight != e2.weight) return e1.weight < e2.weight;
	if (e1.u != e2.u) return e1.u < e2.u;
	else return e1.v < e2.v;
}

int Find(int i)
{
	int temp = i;
	while (vertax[temp].parent != temp) {
		temp = vertax[temp].parent;
	}
	return vertax[temp].parent;
}

void Union(int x, int y)
{
	int rootX = Find(x);
	int rootY = Find(y);

	if (vertax[rootX].rank < vertax[rootY].rank)
		vertax[rootX].parent = rootY;
	else if (vertax[rootX].rank > vertax[rootY].rank)
		vertax[rootY].parent = rootX;

	else
	{
		vertax[rootY].parent = rootX;
		vertax[rootX].rank++;
	}
}

void MST()
{
	Edge* result = new Edge[N];
	int e = 0;

	for (int i = 0; i < N; i++) {
		vertax[i].parent = i;
		vertax[i].rank = 0;
	}
	for (int i = 0; i < M; i++) {
		if (e == N) break;
		Edge next_edge = edge.at(i);
		int x = Find(next_edge.u);
		int y = Find(next_edge.v);
		if (x != y) {
			result[e++] = next_edge;
			Union(x, y);
		}
	}
	printf("%d\n", e);

	for (int i = 0; i < e; i++)
	{
		printf("%d %d %d\n", result[i].u, result[i].v, result[i].weight);
	}
	return;
}

int main()
{
	Edge temp;
	scanf("%d %d", &N, &M);

	for (int i = 0; i < M; i++) {
		int x, y, w;
		Edge e;
		scanf("%d%d%d", &x, &y, &w);
		x, y;
		e.u = x;
		e.v = y;
		e.weight = w;
		edge.push_back(e);
	}

	sort(edge.begin(), edge.end(), compare);

	MST();

	return 0;
}