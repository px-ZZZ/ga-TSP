#include"my_ga.hpp"
#include"my_tsp.hpp"

#include<vector>
#include<random>
#include<algorithm>


//选择
//锦标赛：len是一串路径长度；k是锦标赛规模，指的是挑k个人出来打比赛；rng是全局随机数生成器(传引用)，返回最短路径的下标
//len[i]对应了种群中第i-1个路径的长度，所以i是下标，指代一个tour
int SelectByTournament(const std::vector<double>& len,int k, std::mt19937& rng)
{
	int n = static_cast<int>(len.size());
	std::uniform_int_distribution<int> u(0, n - 1);
	int BestIdx = u(rng);
	for (int i = 1;i < k;i++)
	{
		int Challenger = u(rng);
		if (len[BestIdx] > len[Challenger])
			BestIdx = Challenger;
	}
	return BestIdx;
}

//轮盘赌
//三种适应度
int SelectByRoulette_1(const std::vector<double>& len, int k, std::mt19937& rng)		//倒数取1次方
{
	(void)k;
	const int n = static_cast<int>(len.size());
	double total = 0.0;
	for (double v : len)
		total += 1.0 / v;
	std::uniform_real_distribution<double> u(0.0, total);
	double ptr = u(rng);										//轮盘指针
	for (int i = 0;i < n;i++)
	{
		ptr -= 1.0 / len[i];
		if (ptr <= 0.0)
			return i;
	}
	return n - 1;
}
int SelectByRoulette_2(const std::vector<double>& len, int k, std::mt19937& rng)		//倒数取2次方
{
	(void)k;
	const int n = static_cast<int>(len.size());
	double total = 0.0;
	for (double v : len)
		total += 1.0 / (v*v);
	std::uniform_real_distribution<double> u(0.0, total);
	double ptr = u(rng);										//轮盘指针
	for (int i = 0;i < n;i++)
	{
		ptr -= 1.0 / (len[i]*len[i]);
		if (ptr <= 0.0)
			return i;
	}
	return n - 1;
}
int SelectByRoulette_rank(const std::vector<double>& len, int k, std::mt19937& rng)
{
	(void)k;
	const int n = static_cast<int>(len.size());
	std::vector<int> indexArr(len.size());
	for (int i = 0;i < n;i++)	indexArr[i] = i;
	//排序方法：随机取下标a和b，若第a个长度小（返回ture），那么就让indexArr里面的a（既是元素也是下标）排在前面
	//[&]指的是以引用方式捕获外部变量，用来获取len的数据
	std::sort
	(
		indexArr.begin(), indexArr.end(), [&](int a, int b) 
		{
			return len[a] < len[b];
		}
	);
	int total = n * (n - 1) / 2;
	std::uniform_int_distribution<int> u(1, total);
	int ptr = u(rng);
	for (int rank = 0;rank < n;rank++)			//rank表示第几名优，第0名的权重是总数-0
	{
		int weight = n - rank;
		ptr -= weight;
		if (ptr <= 0)
			return indexArr[rank];				//rank是排名，而indexArr[rank]存的是第rank名原本在len中的下标
	}
	return indexArr[n - 1];
}

//精英保留
std::vector<int> Elitism(const std::vector<double>& len,int m)
{
	const int n = static_cast<int>(len.size());
	if (m > n)	m = n;
	std::vector<int> indexArr(len.size());
	for (int i = 0;i < n;i++)	indexArr[i] = i;
	std::vector<int> arr;
	arr.reserve(len.size());
	std::partial_sort
	(
		indexArr.begin(),
		indexArr.begin() + m,
		indexArr.end(),
		[&](int a, int b) 
		{
			return len[a] < len[b]; 
		}
	);
	for (int i = 0;i < n && i < m;i++)
		arr.push_back(indexArr[i]);
	return arr;
}


//交叉
//顺序交叉						len1和len2是路径父母，rng是全局随机数，返回一个子代路径数组
std::vector<int> OrderCrossover(const std::vector<int>& tour1, const std::vector<int>& tour2, std::mt19937& rng)
{
	const int n = static_cast<int>(tour1.size());
	std::uniform_int_distribution<int> u(0, n - 1);
	int pre = u(rng);
	int las = u(rng);
	if (pre > las)
		std::swap(pre, las);

	std::vector<int> child(n, -1);		//子路径数组
	std::vector<char> used(n, 0);		//检测某城市是否已填

	for (int i = pre;i <= las;i++)		//先填部分的tour1
	{
		child[i] = tour1[i];
		used[tour1[i]] = 1;
	}
	int index = (las + 1) % n;			//依旧成环防越界
	//index用来填子代，填了才自增
	//k用来遍历tour2父代，无论填不填都要自增
	for (int k = 0;k < n;k++)
	{
		const int city = tour2[(k + las + 1) % n];
		if (used[city])		//已用不填
			continue;
		child[index] = city;
		used[city] = 1;
		index = (index + 1) % n;
	}
	return child;
}


//变异
//交换变异
void Mutate_Swap(std::vector<int>& tour, const TspInstance& inst, std::mt19937& rng)
{
	const int n = static_cast<int>(tour.size());
	std::uniform_int_distribution<int> u(0, n - 1);
	int i = u(rng);
	int j = u(rng);
	std::swap(tour[i], tour[j]);
}
//2-optvoid变异
void Mutate_TwoOpt(std::vector<int>& tour, const TspInstance& inst, std::mt19937& rng)
{
	//随机挑选第i和第j个元素，对i,i+1,j,j+1进行翻转
	const int n = static_cast<int>(tour.size());
	std::uniform_int_distribution<int> u(0, n - 1);
	int i = u(rng);
	int j = u(rng);
	if (i > j)	std::swap(i, j);		//保证i小
	if (j - i < 2)	return;				//j至少是i+2才有意义
	std::reverse(tour.begin() + i + 1, tour.begin()+j + 1);		//左闭右开，从i+1到j的调换
}
//2-opt+贪心变异
void Mutate_TwoOptGreedy(std::vector<int>& tour,const TspInstance& inst, std::mt19937& rng)
{
	const int trials = 30;										//尝试的贪心次数

	const int n = static_cast<int>(tour.size());
	std::uniform_int_distribution<int> u(0, n - 1);
	
	double BestDelta = 0.0;
	int Best_i = -1;
	int Best_j = -1;
	for (int step = 0;step < trials;step++)						//在试验次数内找出最贪心的一组
	{
		int i = u(rng);
		int j = u(rng);
		if (i > j)	std::swap(i, j);
		if (j - i < 2)	continue;
		const int a = tour[i];
		const int b = tour[i + 1];
		const int c = tour[j];
		const int d = tour[(j + 1) % n];
		const double delta = inst.distance(a, c) + inst.distance(b, d)
			- inst.distance(a, b) - inst.distance(c, d);
		if (delta < BestDelta)
		{
			BestDelta = delta;
			Best_i = i;
			Best_j = j;
		}
	}
	if (Best_i>=0)
		std::reverse(tour.begin() + Best_i + 1, tour.begin() + Best_j + 1);		//只换一次
	else
	{
		int i = u(rng);
		int j = u(rng);
		if(i!=j)
			std::swap(tour[i], tour[j]);											//随机交换兜底
	}
}
