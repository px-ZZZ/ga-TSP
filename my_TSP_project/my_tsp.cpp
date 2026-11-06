//tsp的cpp文件
//目标：实例化，并生成一个基于临近距离和2-opt算法的基准解

#include "my_tsp.hpp"
#include<cmath>
#include<stdexcept>
#include<random>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<iomanip>


void TspInstance::build()			//将二维矩阵压扁为一维矩阵
{
	n_ = static_cast<int>(cities_.size());
	d_.assign(cities_.size() * cities_.size(), 0.0);
	for (int i = 0;i < n_;i++)
	{
		for (int j = 0;j < n_;j++)
		{
			if (i == j)
				continue;
			double dx = cities_[i].x - cities_[j].x;
			double dy = cities_[i].y - cities_[j].y;
			double dis = sqrt(dx * dx + dy * dy);
			d_[static_cast<size_t>(i) * n_ + j] = dis;
		}
	}
}

double TspInstance::LengthOfTour(const std::vector<int>& tour) const				//算路径总长度
{
	double total = 0.0;
	for (size_t k = 0;k < tour.size();k++)
	{
		const int i = tour[k];
		const int j = tour[(k + 1) % tour.size()];
		total += distance(i, j);
	}
	return total;
}

TspInstance TspInstance::RandomGenerate(int n, unsigned seed, double scale)			//随机生成城市
{
	if (n < 3)
		throw std::invalid_argument("城市数至少为3");

	std::mt19937 rng(seed);
	std::uniform_real_distribution<double> u(0.0, scale);//scale默认1000.0

	TspInstance inst;
	inst.cities_.resize(static_cast<size_t>(n));
	for (auto& c : inst.cities_)
	{
		c.x = u(rng);
		c.y = u(rng);
	}
	inst.build();
	return inst;
}

void TspInstance::SaveCSV(const std::string& path) const							//存入csv
{
	std::ofstream out(path);
	if (!out)
		throw std::invalid_argument("无效的输出路径：" + path);
	for (const auto& c : cities_)
		out << c.x << "," << c.y << '\n';
}

TspInstance TspInstance::FromFile(const std::string& path)							//从文件读取城市
{
	std::ifstream in(path);
	if (!in)
		throw std::runtime_error("无效的读取路径：" + path);
	TspInstance inst;
	std::string line;
	while (getline(in, line))
	{
		if (line.empty())
			continue;
		std::stringstream str_in(line);
		std::string str_x;
		std::string str_y;
		if (!getline(str_in, str_x, ','))	continue;
		if (!getline(str_in, str_y, ','))	continue;
		City c;
		c.x = std::stod(str_x);
		c.y = std::stod(str_y);
		inst.cities_.push_back(c);
	}
	if (inst.cities_.size() < 3)
		throw std::runtime_error("文件里的城市少于3");
	return inst;
}

std::vector<int> NearestNeighbor(const TspInstance& inst)	//生成临近最短的一条路径
{
	const int n = inst.size();
	std::vector<int> tour;
	tour.reserve(static_cast<size_t>(n));
	std::vector<char> visited(static_cast<size_t>(n),0);	//visited数组用于记录城市有没有被访问

	int cur = 0;											//以0为起点
	tour.push_back(cur);
	visited[cur] = 1;

	for (int i = 1;i < n;i++)
	{
		int BestCity = -1;
		double BestLength = 1e18;
		for (int j = 0;j < n;j++)
		{
			if (visited[j])
				continue;
			const double d = inst.distance(cur, j);
			if (d < BestLength)								//每一次都找当前最短的路径（贪心）
			{
				BestLength = d;
				BestCity = j;
			}
		}
		visited[BestCity] = 1;
		tour.push_back(BestCity);
		cur = BestCity;
	}
	return tour;
}

double TwoOptSearch(const TspInstance inst, std::vector<int>& tour, int maxRounds)	//将路径优化后返回它的长度
{
	const size_t n = static_cast<size_t>(tour.size());
	for (int step = 0;step < maxRounds;step++)
	{
		bool improved=0;
		for (int i = 0;i < n - 1;i++)
		{
			for (int j = i + 2;j < n;j++)
			{
				if (i == 0 && j == n - 1)
					continue;
				const int a = tour[i], b = tour[i + 1];
				const int c = tour[j], d = tour[(j + 1) % n];
				const double delta = inst.distance(a, c) + inst.distance(b, d)
					- inst.distance(a, b) - inst.distance(c, d);
				if (delta < -1e-9)
				{
					std::reverse(tour.begin() + i + 1, tour.begin() + j + 1);
					improved = 1;
				}
			}
		}
		if (!improved)
			break;
	}
	return inst.LengthOfTour(tour);
}