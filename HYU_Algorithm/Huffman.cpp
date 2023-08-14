// 2019094511_±Ë¡ÿ«•_12838
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

struct Node {
	int id, num;
	int l, r;
	bool operator>(const Node& rns) {
		return this->num > rns.num;
	}
	friend Node operator+(const Node& lns, const Node& rns);
};

Node operator+(const Node& x, const Node& y) {
	Node node = { 0, x.num + y.num, x.id, y.id };
	return node;
}

void Heapify_Down(Node node[], int f, int N) {
	int l, r, min;
	Node temp = node[f];
	while (f <= N) {
		l = 2 * f, r = l + 1;
		if (r <= N && node[l] > node[r]) {
			min = r;
		}
		else if (l <= N) {
			min = l;
		}
		else break;

		if (temp > node[min]) {
			node[f] = node[min];
		}
		else break;

		f = min;
	}
	node[f] = temp;
}

void Heapify_Up(Node node[], int f, int N) {
	int p = f;
	Node temp = node[f];
	while ((p /= 2) >= 1) {
		if (node[p] > temp) {
			node[f] = node[p];
		}
		else break;

		f = p;
	}
	node[f] = temp;
}

Node Extract_Min(Node node[], int& N) {
	Node ret = node[1];
	node[1] = node[N--];
	Heapify_Down(node, 1, N);
	return ret;
}

void Push(Node node[], const Node& n, int& N) {
	node[++N] = n;
	Heapify_Up(node, N, N);
}

void Heapify(Node node[], int N) {
	for (int i = N / 2; i >= 1; --i) {
		Heapify_Down(node, i, N);
	}
}

int Get_Size(const Node& root, Node node[], int level) {
	if (root.l == 0 && root.r == 0) {
		return level * root.num;
	}
	int sum = 0;
	if (root.l != 0) {
		sum += Get_Size(node[root.l], node, level + 1);
	}
	if (root.r != 0) {
		sum += Get_Size(node[root.r], node, level + 1);
	}
	return sum;
}

int main() {
	int i, N;
	int* arr = NULL;
	char ch[5];

	scanf("%d", &N);
	arr = new int[N + 1];
	for (i = 1; i <= N; i++) {
		scanf("%s %d", ch, &arr[i]);
	}
	scanf("%d", &arr[0]);

	Node* item = new Node[2 * N + 1];
	Node* node = new Node[N + 1];

	int item_size = N;
	int node_size = N;
	
	for (i = 1; i <= N; i++) {
		item[i].id = i;
		item[i].num = arr[i];
		item[i].l = item[i].r = 0;
		node[i] = item[i];
	}

	Heapify(node, node_size);

	while (node_size > 1) {
		Node x = Extract_Min(node, node_size);
		Node y = Extract_Min(node, node_size);
		Node z = x + y;
		z.id = ++item_size;
		item[item_size] = z;
		Push(node, z, node_size);
	}

	printf("%d\n", (static_cast<int>(log2(N)) + 1) * arr[0]);
	printf("%d\n", node[1].id <= N ? node[1].num : Get_Size(node[1], item, 0));

	delete[] arr;
	delete[] node;
	delete[] item;
	return 0;
}