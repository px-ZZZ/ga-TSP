#include "my_tsp.hpp"
#include"my_ga.hpp"

#include<iostream>
#include<random>
#include<algorithm>
#include<vector>
#include<functional>
#include<string>
#include<fstream>
#include<iomanip>
#include<ctime>
#include<chrono>
#include<set>

// 获取当前时间，格式为 "YYYYMMDD_HHMMSS"
std::string getTimestamp()
{
	// 1. 获取当前系统时间点
	auto now = std::chrono::system_clock::now();
	// 2. 转换为 time_t (C 风格时间)
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	// 3. 转换为本地时间结构体
	std::tm tm_buf;
	localtime_s(&tm_buf, &now_time);
	std::ostringstream oss;
	// %Y年 %m月 %d日 %H时 %M分 %S秒
	oss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
	return oss.str();
}

//搭建“控制面板”
// 1. 定义选择算子的插槽
// 参数：					返回：选中的下标	len数组，		 k值，	 随机数引擎。
using SelectFunc = std::function<int(const std::vector<double>&, int, std::mt19937&)>;
// 2. 定义交叉算子的插槽
// 参数：							返回：新子代              父1，						父2，			随机数引擎。
using CrossoverFunc = std::function<std::vector<int>(const std::vector<int>&, const std::vector<int>&, std::mt19937&)>;
// 3. 定义变异算子的插槽
// 参数：				返回：void		要修改的路线，			距离实例，		随机数引擎。
using MutateFunc = std::function<void(std::vector<int>&, const TspInstance&, std::mt19937&)>;


int main(int argc, char** argv)
{
	std::cout << "成功执行一次实验" << std::endl;
	int n = 50;					//城市数
	int popSize = 100;			//种群规模
	int gens = 500;				//代数
	int seed = 4;				//随机数种子

	int k = 3;					//锦标赛规模
	double pc = 0.9;			//交叉率
	double pm = 0.9;			//变异率
	int eli = 2;				//精英数

	std::string selName = "tournament";			//选择方法
	std::string crossName = "OX";				//交叉方法
	std::string mutName = "swap";				//变异方法

	//读取参数
	for (int i = 1;i < argc;i++)
	{
		std::string arg = argv[i];
		if (arg == "--sel" && i + 1 < argc)	selName = argv[++i];						//选择方法
		else if (arg == "--cross" && i + 1 < argc)	crossName = argv[++i];				//交叉方法
		else if (arg == "--mut" && i + 1 < argc)	mutName = argv[++i];				//变异方法
		else if (arg == "--k" && i + 1 < argc) k = std::stoi(argv[++i]);				//锦标赛规模
		else if (arg == "--pc" && i + 1 < argc) pc = std::stod(argv[++i]);				//交叉率
		else if (arg == "--pm" && i + 1 < argc) pm = std::stod(argv[++i]);				//变异率
		else if (arg == "--n" && i + 1 < argc) n = std::stoi(argv[++i]);				//城市数
		else if (arg == "--popS" && i + 1 < argc) popSize = std::stoi(argv[++i]);		//种群规模
		else if (arg == "--gens" && i + 1 < argc) gens = std::stoi(argv[++i]);			//代数
		else if (arg == "--seed" && i + 1 < argc) seed = std::stoi(argv[++i]);			//随机数种子
		else if (arg == "--eli" && i + 1 < argc) eli = std::stoi(argv[++i]);			//精英数
	}

	//给结果文件取名
	std::string nowtime = getTimestamp();
	std::string FilePath = "E:\\CodeTools\\Project\\my_TSP_project\\results\\";
	std::string FileName = selName + "_" + crossName + "_" + mutName + "_" + std::to_string(seed) + "_"+nowtime;
	std::string FullFile = FilePath + FileName+ ".csv";
	std::string FileGragh = FilePath + FileName + "_gragh" + ".csv";

	std::ofstream out(FullFile, std::ios::app);											//存入当前策略
	if (!out)
		throw std::invalid_argument("无效的输出结果路径：" + FullFile);
	out << selName << "_" << crossName << "_" << mutName <<'\n';
	out << "pc= " <<std::fixed<<std::setprecision(2)<< pc << "," << " pm= " << pm << "," << " seed= " << std::to_string(seed)<<'\n';
	if (eli != 2)
		out << "eli= " << std::to_string(eli) << '\n';
	if (popSize != 100)
		out << "popSize= " << std::to_string(popSize) << '\n';
	if (gens != 500)
		out << "gens= " << std::to_string(gens) << '\n';

	//初始化插槽变量（待插入具体函数名）
	SelectFunc selectFunc;
	CrossoverFunc crossFunc;
	MutateFunc mutateFunc;

	//选择
	if (selName == "tournament")
	{
		selectFunc = SelectByTournament;
		out << "tournament_k= " << k << '\n';
	}
	else if (selName == "rank")
		selectFunc = SelectByRoulette_rank;
	else if (selName == "r1")
		selectFunc =  SelectByRoulette_1;
	else if (selName == "r2")
		selectFunc = SelectByRoulette_2;
	else
		throw std::invalid_argument("无效的选择策略：" + selName);

	//交叉
	if (crossName == "OX")
		crossFunc = OrderCrossover;
	else
		throw std::invalid_argument("无效的交叉策略：" + crossName);

	//变异
	if (mutName == "swap")
		mutateFunc = Mutate_Swap;
	else if (mutName == "2opt")
		mutateFunc = Mutate_TwoOpt;
	else if (mutName == "2optG")
		mutateFunc = Mutate_TwoOptGreedy;
	else
		throw std::invalid_argument("无效的变异策略：" + mutName);

	std::mt19937 rng(seed);
	TspInstance inst = TspInstance::RandomGenerate(n, 42);		//以42作为随机数种子实例化一个对象(重复实验都不改变这个值)

	std::vector<int> base(n);										//初始化路径：从0一直走到49号城市
	for (int i = 0;i < n;i++)
		base[i] = i;

	std::uniform_real_distribution<double> r(0.0, 1.0);		//0到1之间的随机数，用于判断是否变异和交叉
	double RandNum = r(rng);

	std::vector<std::vector<int>> pop(popSize);						//pop即为一个100行，50列的二维数组，用于记录100个路径
	//！每个pop[i]都是一个tour！

	for (int i = 0;i < popSize;i++)
	{
		pop[i] = base;												//让pop的某一行基于base随机打乱
		std::shuffle(pop[i].begin(), pop[i].end(), rng);			//从而生成100个路径的个体
	}

	std::vector<double> len(popSize);								//初步算出初始种群的路径长短
	for (int i = 0;i < popSize;i++)
	{
		len[i] = inst.LengthOfTour(pop[i]);
	}

	double FirstBest = *std::min_element(len.begin(), len.end());	//存入起始的最优路径
	out << '\n';
	out << "gen" << "," << "best" << '\n';
	out << -1 << "," <<std::fixed<<std::setprecision(4)<< FirstBest << '\n';

	int StopGen = 0;												//判断第几代收敛
	double Shortest = 1e9;
	int count = 0;													//连续多少代不变判断为收敛
	int finalstop = 0;
	
	std::ofstream outGragh(FileGragh);
	if (!outGragh)
		throw std::invalid_argument("无效的输出图表路径：" + FileGragh);
	outGragh << "gen" << "," << "best" << '\n';

	for (int g = 0;g < gens;g++)
	{
		std::vector<std::vector<int>> nextpop;						//子种群，同样是一个二维数组
		nextpop.reserve(popSize);									//为什么要用reserve而不是提前定大小？为了后面用大小作为循环条件

		//0.精英保留
		std::vector<int> eliteIndex(eli);
		eliteIndex = Elitism(len, eli);
		for (int i = 0;i < eli;i++)
			nextpop.push_back(pop[eliteIndex[i]]);

		while (nextpop.size() < popSize)
		{
			
			//1.选择
			int idx1 = selectFunc(len, k, rng);
			int idx2 = selectFunc(len, k, rng);

			std::vector<int> child;									//子代

			//2.交叉
			if (RandNum < pc)										//满足交叉率
			{
				child = crossFunc(pop[idx1], pop[idx2], rng);		//父代是两个tour
			}
			else													//不发生交叉时，子代直接等于其中一个父代
			{
				child = pop[idx1];
			}

			//3.变异
			if (RandNum < pm)										//满足变异率
			{
				mutateFunc(child,inst,rng);
			}

			nextpop.push_back(child);								//相当于在二维数组里直接写了一行（一个tour）
		}

		pop = nextpop;												//新老换代
		for (int i = 0;i < popSize;i++)
		{
			len[i] = inst.LengthOfTour(pop[i]);						//重新算路径长度
		}

		double currentBest = *std::min_element(len.begin(), len.end());
		out << g << "," << std::fixed << std::setprecision(4) << currentBest << '\n';			//每一代结果存入文件
		outGragh << g << "," << std::fixed << std::setprecision(4) << currentBest << '\n';

		if (currentBest < Shortest)									//判断是否收敛
		{
			Shortest = currentBest;
			StopGen = g;
			count = 0;
		}
		else 
			count++;
		if (count >= 50)
		{
			finalstop = StopGen;
		}


		///控制台测试区
		// 1. 看看种群多样性（有多少个不同的个体）
		//std::set<std::vector<int>> unique_pop(pop.begin(), pop.end());
		//std::cout << "第 " << g << " 代，不同个体数量: " << unique_pop.size() << "\n";

		//// 2. 看看最差的那个有多差
		//double worst = *std::max_element(len.begin(), len.end());
		//std::cout << "当前代最好: " << currentBest << "，最差: " << worst << "\n";
		//======================================================
	}
	std::vector<int> NearestTour = NearestNeighbor(inst);
	//std::cout << "基于临近最短的2-opt优化结果：" << TwoOptSearch(inst, NearestTour, 50)<<'\n';
	out << "stop improve at gen:" << std::to_string(finalstop) << '\n';
	

	out << '\n';
	out << "standard"<<"," << std::fixed << std::setprecision(4) << TwoOptSearch(inst, NearestTour, 50) << '\n';
	out.close();
	return 0;
}