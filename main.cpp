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
#include <random>
#include <algorithm>

#define REP(i, n) for (int i = 0; i < n; i++)
#define ALL(v) (v).begin(), (v).end()

using namespace std;

typedef long long ll;

// TODO: 得意分野を考慮
// TODO: 依存関係があるタスクを優先
// TODO: union-findして，クリティカルパスが大きいものから優先
// TODO: ufじゃなくて，DAGの最長経路問題に帰着できそう
// TODO: タスク交換したほうがいいなら交換，みたいなのやりたい
// TODO: 思ってたより早く終わったらそのタスクの形で補正してあげる
// TODO: 過去にやった全タスクに関して，現在のスキルから計算した推定値と実際に掛かった日付
// スキルをいじって，推定値が実際に掛かった日付に近づいたら良い
// TODO: ヒルクライム法，やきなまし法
struct TaskMemberPair
{
	ll taskId;
	ll memberId;
};

ll depth(ll root); // rootからの最長経路
ll getFree(vector<bool> &free, ll task, ll days);
ll getTask(set<ll> &tasks, vector<ll> &u, vector<ll> &v, ll days);
bool canDo(ll task, vector<ll> &u, vector<ll> &v);
vector<TaskMemberPair> assign(vector<bool> &free, set<ll> &tasks, vector<ll> &u, vector<ll> &v, ll days);
template <class T>
void printSet(set<T> &s)
{
	cout << "set: ";
	for (auto x : s)
	{
		cout << x << " ";
	}
	cout << endl;
}

template <class T>
void printVec(vector<T> &v)
{
	cout << "vector: ";
	for (auto x : v)
	{
		cout << x << " ";
	}
	cout << endl;
}

template <class T>
void printErrVec(vector<T> &v)
{
	cerr << "vector: ";
	for (auto x : v)
	{
		cerr << x << " ";
	}
	cerr << endl;
}
ll llabs(ll x)
{
	return x < 0 ? -x : x;
}

// variables
set<ll> done;
vector<ll> allocTask;				// id: workerId, elem: taskId
vector<ll> workStartDate;		// id: workerId, elem: workを始めた日
vector<ll> taskWeight;			// id: taskId, elem: taskの重み(どれくらい技能を必要とされるか)
vector<double> skills;			// id: workerId, elem: 技能
vector<vector<ll>> skills2; // id: workerId, elem_i: 技能i
vector<vector<ll>> d;
vector<vector<ll>> dep; // dep[i] = {j,k,h} i→j, i→k, i→h
vector<ll> maxLength;		// id: taskId, elem: taskIdからの最長経路の長さ
ll taskNum, workerNum;
vector<ll> acctualyDays;						// 実際にタスクにかかった日数
vector<vector<ll>> doneWorkerTasks; // doneWorkerTasks[member_id] = {task_ids}

int main(int argc, char const *argv[])
{
	// 乱数生成器を用意する
	std::random_device seed_gen;
	std::mt19937 engine{seed_gen()};
	// n: タスク数, m: メンバー, k: 技能, r: 依存関係
	ll n, m, k, r;
	cin >> n >> m >> k >> r;
	taskNum = n;
	workerNum = m;
	d = vector<vector<ll>>(n, vector<ll>(k));
	vector<bool> free(m, true);
	allocTask = vector<ll>(m, -1);
	workStartDate = vector<ll>(m, 0);
	skills = vector<double>(m, 0);
	skills2 = vector<vector<ll>>(m, vector<ll>(k, 0));
	taskWeight = vector<ll>(n, 0);
	dep = vector<vector<ll>>(n, vector<ll>());
	maxLength = vector<ll>(n, -1);
	acctualyDays = vector<ll>(n, 0);
	doneWorkerTasks = vector<vector<ll>>(m, vector<ll>());
	// tasks: 残タスク0~n-1
	done = set<ll>();
	set<ll> tasks;
	REP(i, n)
	{
		tasks.insert(i);
	}

	vector<ll> u(r);
	vector<ll> v(r);
	REP(i, n)
	{
		ll sum = 0;
		REP(j, k)
		{
			cin >> d[i][j];
			sum += d[i][j];
		}
		taskWeight[i] = sum;
	}
	vector<bool> isRoot(n, true);
	REP(i, r)
	{
		cin >> u[i] >> v[i];
		u[i] = u[i] - 1;
		v[i] = v[i] - 1;
		dep[u[i]].push_back(v[i]);
		isRoot[v[i]] = false;
	}
	// 各treeのstartからendまでの長さを求める(とりあえず辺の重みは1)
	{
		REP(i, n)
		{
			if (isRoot[i])
			{
				maxLength[i] = depth(i);
			}
		}
	}

	ll days = 0;
	while (true)
	{
		days++;
		// start
		vector<TaskMemberPair> assigned = assign(free, tasks, u, v, days);
		if (assigned.size() == 0)
		{
			cout << 0 << endl;
		}
		else
		{
			// m member_i task_i
			cout << assigned.size();
			for (auto x : assigned)
			{
				cout << " " << x.memberId + 1 << " " << x.taskId + 1;
			}
			cout << endl;
		}
		// closed
		ll finished_num;
		cin >> finished_num;
		if (finished_num == 0)
		{
			cerr << -1 << endl;
			continue;
		}
		else if (finished_num == -1)
		{
			// cerr << days << " " << "finished!!!" << endl;
			break;
		}
		ll member_id;
		REP(i, finished_num)
		{
			cin >> member_id;
			member_id -= 1;
			free[member_id] = true;
			ll doneTaskId = allocTask[member_id];
			done.insert(doneTaskId);
			cerr << doneTaskId << " ";
			// printSet(done);
			allocTask[member_id] = -1;

			// タスク終了にかかった期間を測る
			ll workDays = days - workStartDate[member_id] + 1;
			acctualyDays[doneTaskId] = workDays;
			doneWorkerTasks[member_id].push_back(doneTaskId);
			// 技能を更新
			skills[member_id] = (skills[member_id] + taskWeight[doneTaskId] / workDays) / 2.0;
			if (workDays == 1)
			{
				REP(i, skills2[member_id].size())
				{
					skills2[member_id][i] = max(d[doneTaskId][i], skills2[member_id][i]);
				}
			}
			else
			{
				REP(i, skills2[member_id].size())
				{
					skills2[member_id][i] = max(d[doneTaskId][i] - workDays, skills2[member_id][i]);
				}
			}
			// printVec(skills2[member_id]);
		}
		cerr << endl;
		if (days > 500 && days % 10 == 0)
		{
			// ヒルクライム法
			REP(i, m)
			{
				// ひとりずつ更新
				ll doneTasksSize = doneWorkerTasks[i].size();
				vector<ll> estimates = vector<ll>(n, 0);
				for (auto doneTaskId : doneWorkerTasks[i])
				{
					//まずは推定コストを計算
					ll estimate = 0;
					REP(j, k)
					{
						estimate += max(ll(0), d[doneTaskId][j] - skills2[i][j]);
					}
					estimates[doneTaskId] = estimate;
				}
				// とりあえず100回くらい
				REP(hh, 10+done.size()/10)
				{
					struct SkillIDUpdateParamPair
					{
						ll skillId;
						ll updateParam;
						ll cost;
					};
					auto compare = [](SkillIDUpdateParamPair a, SkillIDUpdateParamPair b)
					{
						return a.cost > b.cost; // asc
					};
					priority_queue<SkillIDUpdateParamPair, vector<SkillIDUpdateParamPair>, decltype(compare)> pq{compare};
					REP(h, 100)
					{
						ll before = [&estimates, i]
						{
							ll tmpSum = 0;
							for (auto ti : doneWorkerTasks[i])
							{
								tmpSum += llabs(acctualyDays[ti] - estimates[ti]);
							}
							return tmpSum;
						}();
						// ll randomTaskId = doneWorkerTasks[engine()%doneWorkerTasks[i].size()];
						ll randomSkillId = engine() % k;
						vector<ll> afterEstimates = vector<ll>(n, 0);
						ll updateParam = 0;
						for (auto doneTaskId : doneWorkerTasks[i])
						{
							//まずは推定コストを計算
							ll estimate = 0;
							REP(j, k)
							{
								if (j == randomSkillId)
								{
									if (days > 0)
									{
										if (rand() % 5 == 0)
										{
											updateParam = -1;
										}
										else
										{
											updateParam = 1;
										}
									}
									else
									{
										updateParam = 1;
									}
									estimate += max(ll(0), d[doneTaskId][j] - skills2[i][j] - updateParam);
								}
								else
								{
									estimate += max(ll(0), d[doneTaskId][j] - skills2[i][j]);
								}
							}
							afterEstimates[doneTaskId] = estimate;
						}
						ll after = [&afterEstimates, i]
						{
							ll tmpSum = 0;
							for (auto ti : doneWorkerTasks[i])
							{
								tmpSum += llabs(acctualyDays[ti] - afterEstimates[ti]);
							}
							return tmpSum;
						}();
						// cerr << before << " " << after << endl;
						if ((before > after) || ((rand() % 2000 < 2000 - days - hh*50) && (before - after > - 30)))
						{ // 更新後小さくなっていれば更新採用
							// REP(jj, estimates.size())
							// {
							// 	estimates[jj] = afterEstimates[jj];
							// }
							// skills2[i][randomSkillId] += updateParam;
							// cerr << after << " " << before - after << endl;
							pq.push({randomSkillId, updateParam, after});
							// cerr << "bool" << endl;
						}
					}
					if (pq.empty())
					{
						continue;
					}
					auto x = pq.top();
					// cerr << x.cost << endl;
					skills2[i][x.skillId] += x.updateParam;
				}
			}
		}
		REP(i, m)
		{
			if (days == 701)
			{
				// cerr << skills[i] << endl; //jikken
			}
			// printErrVec(skills2[i]);
		}
	}
	REP(i, m)
	{
		// printErrVec(skills2[i]);
	}
	return 0;
}

// return memberId
ll getFree(vector<bool> &free, ll task, ll days)
{
	struct MemberSkillPair
	{
		ll memberId;
		ll cost;
		ll minus; // 過剰戦力を投入しないように→効果なし
	};
	auto compare = [](MemberSkillPair a, MemberSkillPair b)
	{
		if (a.cost == 0 && b.cost == 0)
		{
			return a.minus < b.minus; // 降順
		}
		return a.cost > b.cost; // 昇順
	};
	priority_queue<MemberSkillPair, vector<MemberSkillPair>, decltype(compare)> pq{compare};
	// skillでソート
	REP(i, skills.size())
	{
		ll costSum = 0;
		ll minus = 0;
		REP(j, d[task].size())
		{
			minus += d[task][j] - skills2[i][j];
			costSum += max(ll(0), d[task][j] - skills2[i][j]);
		}
		pq.push({memberId : i, cost : costSum, minus : minus});
	}
	while (!pq.empty())
	{
		auto x = pq.top();
		pq.pop();
		if (free[x.memberId])
		{
			free[x.memberId] = false;
			return x.memberId;
		}
	}
	// 全員空いてなかったら-1
	return -1;
}

ll getTask(set<ll> &tasks, vector<ll> &u, vector<ll> &v, ll days)
{
	if (tasks.size() == 0)
	{
		return -1;
	}
	struct TaskPriorityPair
	{
		ll taskId;
		ll priority;
	};
	auto compare = [](TaskPriorityPair a, TaskPriorityPair b)
	{
		return a.priority < b.priority; // 降順
	};
	priority_queue<TaskPriorityPair, vector<TaskPriorityPair>, decltype(compare)> pq{compare};
	for (auto value : tasks)
	{
		pq.push({taskId : value, priority : maxLength[value]});
	}
	ll task = -1;
	while (!pq.empty())
	{
		auto value = pq.top();
		pq.pop();
		// 依存関係チェック
		if (canDo(value.taskId, u, v))
		{
			task = value.taskId;
			tasks.erase(task);
			return task;
		}
	}
	return -1;
}

bool canDo(ll task, vector<ll> &u, vector<ll> &v)
{
	REP(i, u.size())
	{
		if (v[i] == task)
		{
			// 依存先がまだ終わっていなかったら
			if (done.find(u[i]) == done.end())
			{
				return false;
			}
		}
	}
	return true;
}

bool busy(vector<bool> &free)
{
	REP(i, free.size())
	{
		if (free[i])
		{
			return false;
		}
	}
	return true;
}

vector<TaskMemberPair> assign(vector<bool> &free, set<ll> &tasks, vector<ll> &u, vector<ll> &v, ll days)
{
	struct MemberSkillPair
	{
		ll memberId;
		ll cost;
		ll minus; // 過剰戦力を投入しないように→効果なし
	};
	auto compare = [](MemberSkillPair a, MemberSkillPair b)
	{
		if (a.cost == 0 && b.cost == 0)
		{
			return a.minus < b.minus; // 降順
		}
		return a.cost > b.cost; // 昇順
	};
	priority_queue<MemberSkillPair, vector<MemberSkillPair>, decltype(compare)> pq{compare};
	vector<vector<ll>> costs = vector<vector<ll>>(workerNum, vector<ll>(taskNum, 0));
	auto ret = vector<TaskMemberPair>();
	while (true)
	{
		// 空いている人がいなかったら終了
		if (busy(free))
		{
			return ret;
		}
		// 開始可能なタスクの取得
		ll task = getTask(tasks, u, v, days);
		if (task == -1)
		{
			// cout << 0 << endl;
			break;
		}
		else
		{
			// skillでソート
			priority_queue<MemberSkillPair, vector<MemberSkillPair>, decltype(compare)> pq{compare};
			REP(i, skills.size())
			{
				ll costSum = 0;
				ll minus = 0;
				REP(j, d[task].size())
				{
					minus += d[task][j] - skills2[i][j];
					costSum += max(ll(0), d[task][j] - skills2[i][j]);
				}
				pq.push({memberId : i, cost : costSum, minus : minus});
				costs[i][task] = costSum;
			}
			// 空いている人を探す
			ll worker = getFree(free, task, days);
			if (worker == -1)
			{
				// cout << 0 << endl;
				break;
			}
			else
			{
				allocTask[worker] = task;
				workStartDate[worker] = days;
				// m member_i task_i
				ret.push_back(TaskMemberPair{taskId : task, memberId : worker});
			}
			// cout << 1 << " " << worker + 1 << " " << task + 1 << endl;
		}
	}
	vector<ll> workers = vector<ll>();
	REP(i, ret.size())
	{
		workers.push_back(ret[i].memberId);
	}
	// タスク交換フェーズ
	return ret;
	REP(i, workers.size())
	{
		REP(j, workers.size())
		{
			if (i == j)
			{
				continue;
			}
			// もしタスクを入れ替えたほうが安い場合
			if (costs[workers[i]][ret[i].taskId] + costs[workers[j]][ret[j].taskId] > 10 + costs[workers[i]][ret[j].taskId] + costs[workers[j]][ret[i].taskId])
			{
				// depthが大きい方のタスクに負担がかかりすぎそうならやめる
				if ((maxLength[ret[i].taskId] > maxLength[ret[j].taskId]) && (costs[workers[i]][ret[i].taskId] < costs[workers[j]][ret[i].taskId]) && (float(maxLength[ret[i].taskId]) / float(maxLength[ret[j].taskId] + 1) > 1.2)) // ゼロ除算にならないように
				{
					continue;
				}
				if ((maxLength[ret[j].taskId] > maxLength[ret[i].taskId]) && (costs[workers[j]][ret[j].taskId] < costs[workers[i]][ret[j].taskId]) && (float(maxLength[ret[j].taskId]) / float(maxLength[ret[i].taskId] + 1) > 1.2))
				{
					continue;
				}
				allocTask[workers[i]] = ret[j].taskId;
				allocTask[workers[j]] = ret[i].taskId;
				auto tmp = ret[i].taskId;
				ret[i].taskId = ret[j].taskId;
				ret[j].taskId = tmp;
			}
		}
	}
	return ret;
}

ll depth(ll root)
{
	if (dep[root].size() == 0)
	{
		return 0;
	}
	ll dp = 0;
	for (auto &d : dep[root])
	{
		if (maxLength[d] == -1)
		{
			maxLength[d] = depth(d);
		}
		dp = max(dp, maxLength[d]);
	}
	return dp + 1;
}
