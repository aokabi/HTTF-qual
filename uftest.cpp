#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <string>
#include <stack>
#include <regex>
#include <numeric>
#include <queue>
#include <set>
 
#define REP(i, n) for (int i = 0; i < n; i++)
#define ALL(v) (v).begin(), (v).end()
 
using namespace std;
 
typedef long long ll;

struct union_find
{
private:
	vector<int> parent;
	vector<int> rank;
	void swap(int *x, int *y)
	{
		int tmp = *x;
		*x = *y;
		*y = tmp;
	}

public:
	int tree_num;
	void init(int nodenum)
	{
		parent.resize(nodenum, 0);
		for (int i = 0; i < nodenum; i++)
			parent[i] = i;
		rank.resize(nodenum, 0);
		tree_num = nodenum;
	}
	int find(int x)
	{
		if (parent[x] == x)
			return x;
		parent[x] = find(parent[x]);
		return parent[x];
	}
	void unite(int x, int y)
	{
		x = find(x);
		y = find(y);
		if (x == y)
			return;
		if (rank[x] < rank[y])
			swap(&x, &y);
		rank[x]++;
		rank[y] = rank[x];
		parent[y] = x;
		tree_num--;
		return;
	}
};

int main(int argc, char const *argv[])
{
	union_find uf;
	uf.init(3);
	uf.unite(1,2);
	cout << uf.tree_num << endl;
	cout << uf.find(0) << uf.find(1) << uf.find(2) << endl;
	return 0;
}
