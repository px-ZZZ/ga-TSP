//tsp头文件
//目标：创建city结构体，定义类名及一些类方法
#pragma once

#include<string>
#include<vector>

struct City 
{
	double x = 0.0;
	double y = 0.0;
};

class TspInstance
{
public:
	TspInstance() = default;
	static TspInstance RandomGenerate(int n, unsigned seed, double scale = 1000.0);		//城市坐标范围0.0-1000.0
	static TspInstance FromFile(const std::string& path);

	int size() const { return static_cast<int>(cities_.size()); }
	const std::vector<City> cities() const { return cities_; }

	double distance(int i, int j) const
	{
		size_t temp = static_cast<size_t>(i);
		return d_[temp * n_ + j];
	}

	double LengthOfTour(const std::vector<int>&tour) const;
	void SaveCSV(const std::string& path) const;

private:
	void build();

	std::vector<City> cities_;
	std::vector<double> d_;
	int n_ = 0;
};

std::vector<int> NearestNeighbor(const TspInstance& inst);								//生成临近最短的一条路径
double TwoOptSearch(const TspInstance inst, std::vector<int>& tour, int maxRounds);		//将路径优化后返回它的长度