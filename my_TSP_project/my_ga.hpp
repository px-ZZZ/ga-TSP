#pragma once

#include"my_tsp.hpp"
#include<random>
#include<vector>


//选择

//锦标赛
int SelectByTournament(const std::vector<double>& len, int k, std::mt19937& rng);


//轮盘赌
//三种适应度
int SelectByRoulette_1(const std::vector<double>& len, int k, std::mt19937& rng);
int SelectByRoulette_2(const std::vector<double>& len, int k, std::mt19937& rng);
int SelectByRoulette_rank(const std::vector<double>& len, int k, std::mt19937& rng);

//精英保留
std::vector<int> Elitism(const std::vector<double>& len,int m);


//交叉

//顺序交叉
std::vector<int> OrderCrossover(const std::vector<int>& tour1, const std::vector<int>& tour2, std::mt19937& rng);


//变异

//交换变异
void Mutate_Swap(std::vector<int>& tour, const TspInstance& inst, std::mt19937& rng);

//2-optvoid
void Mutate_TwoOpt(std::vector<int>& tour, const TspInstance& inst, std::mt19937& rng);

//2-opt+贪心变异
void Mutate_TwoOptGreedy(std::vector<int>& tour, const TspInstance& inst, std::mt19937& rng);