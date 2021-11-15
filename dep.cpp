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
vector<vector<ll>> dep;
ll depth(ll root) {
	if (dep[root].size() == 0) {
		return 0;
	}
	ll dp = 0;
	for (auto& d : dep[root]) {
		dp = max(dp, depth(d));
	}
	return dp+1;
}

int main(int argc, char const *argv[])
{ 
	dep = vector<vector<ll>>(4, vector<ll>());
	dep[0].push_back(2);
	dep[1].push_back(2);
	dep[2].push_back(3);
	cout << depth(0) << depth(1) << endl;
	return 0;
}
