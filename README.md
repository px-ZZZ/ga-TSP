# GA-TSP

用C++实现的遗传算法（Genetic Algorithm,GA）求解旅行商问题（Traveling Saleman Problem,TSP）：4种选择策略、1种交叉算子、三种变异算子、精英保留，全部可在命令行切换，每次实验自动落盘为CSV。

这是一个**学习+实验**性质的项目，重点不在于“跑出最好的解”，而在于“把不同策略放在一起，直观地观察各个算子起到什么作用”。



## 实现清单

| 模块  | 可选内容                                                           |
|:--- |:-------------------------------------------------------------- |
| 选择  | 锦标赛（规模 k 可调）、倒数一次方轮盘赌、倒数二次方轮盘赌、排名轮盘赌、精英保留（个数可调）                |
| 交叉  | 顺序交叉（Order Crossover, OX）                                      |
| 变异  | 交换（swap）、2-opt 反转、2-opt + 贪心                                   |
| 基准  | 最近邻构造（Nearest Neighbor）、2-opt 局部搜索，结果写入 CSV 的 `standard` 行作为对照 |
| 输出  | 可选每次实验一份 CSV（含配置元数据）或 一份只含 `gen,best` 的绘图用 CSV 或 多次实验的最终结果     |



## 目录结构

```text
my_TSP_project/
├── my_TSP_project.slnx        解决方案 (Visual Studio)
├── my_TSP_project/            源码
│   ├── my_tsp.hpp / my_tsp.cpp    实例、距离矩阵、路径长度、最近邻、2-opt
│   ├── my_ga.hpp / my_ga.cpp      选择 / 交叉 / 变异算子
│   ├── my_tsp_main.cpp            命令行解析、实验主循环、CSV 输出
│   └── my_TSP_project.vcxproj     工程文件 (C++20, v145 工具集)
├── results/                   实验输出 (CSV, 按配置分子目录)
├── 排列组合几个策略.txt         参数速查表
└── experiments                 实验的批处理文件
```

## 编译

用 Visual Studio 打开 `my_TSP_project.slnx`，选择 `x64 / Debug`，直接 F5。

- 语言标准：C++20
- 平台工具集：v145（VS 2022 17.10+ 或更新版本）；用较旧的 VS 打开时按提示"重定向项目"即可
- 产物：`x64\Debug\my_TSP_project.exe`
## 运行

```bat
x64\Debug\my_TSP_project.exe --sel tournament --mut swap --seed 1
x64\Debug\my_TSP_project.exe --sel r2 --mut 2optG --pm 0.6 --seed 3 --save gragh
x64\Debug\my_TSP_project.exe --sel rank --mut 2opt --n 100 --gens 1000 --seed 7 --save experiment
```

### 参数

| 参数        | 含义                                       | 默认值          |
| --------- | ---------------------------------------- | ------------ |
| `--sel`   | 选择策略：`tournament` / `r1` / `r2` / `rank` | `tournament` |
| `--k`     | 锦标赛规模（仅 `tournament` 有效）                 | 3            |
| `--eli`   | 精英保留个数                                   | 2            |
| `--cross` | 交叉算子：`OX`                                | `OX`         |
| `--mut`   | 变异算子：`swap` / `2opt` / `2optG`           | `swap`       |
| `--pc`    | 交叉概率                                     | 0.9          |
| `--pm`    | 变异概率                                     | 0.3          |
| `--popS`  | 种群规模                                     | 100          |
| `--gens`  | 迭代代数                                     | 500          |
| `--n`     | 城市数量                                     | 50           |
| `--seed`  | 随机种子                                     | 42           |
| `--save`  | 保存方法                                     | `whole`      |

### 批量跑多个种子

`run_tournament_k2_swap_seed_1,10_exp.bat` 演示了"同一配置换 10 个种子各跑一次"：

```bat
@echo off

cd /d "%~dp0.."

for /L %%i in (1,1,10) do (
    "x64\Debug\my_TSP_project.exe" --sel tournament --mut swap --seed %%i --save experiment
)
pause
```

## 输出格式

每次运行在 `results/` 下生成仅一个文件。

## 已跑的实验

配置：在全默认参数下跑完了3种选择*4种变异=12个结果，10 个随机种子各跑一次。

有初步结论但还没有深入分析


