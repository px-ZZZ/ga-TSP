# GA-TSP：遗传算法求解旅行商问题（C++20）

用 C++ 实现的遗传算法（Genetic Algorithm，GA）求解旅行商问题（Traveling Salesman
Problem，TSP）：4 种选择策略、1 种交叉算子、3 种变异算子、精英保留，全部可在命令行切换，
每次实验自动落盘为 CSV，方便横向对比不同配置的收敛行为。

这是一个**学习 + 实验**性质的项目，重点不在"跑出最好的解"，而在"把不同配置放在同一套
评测下比一比，看清楚每个部件到底起了什么作用"。

## 实现清单

| 模块  | 可选内容                                                           |
| --- | -------------------------------------------------------------- |
| 选择  | 锦标赛（规模 k 可调）、倒数一次方轮盘赌、倒数二次方轮盘赌、排名轮盘赌、精英保留（个数可调）                |
| 交叉  | 顺序交叉（Order Crossover，OX）                                       |
| 变异  | 交换（swap）、2-opt 反转、2-opt + 贪心                                   |
| 基准  | 最近邻构造（Nearest Neighbor）、2-opt 局部搜索，结果写入 CSV 的 `standard` 行作为对照 |
| 输出  | 可选每次实验一份 CSV（含配置元数据）或 一份只含 `gen,best` 的绘图用 CSV  或  多次实验的最终结果   |

 ## 目录结构

```
my_TSP_project/
├── my_TSP_project.slnx              解决方案（Visual Studio）
├── my_TSP_project/                  源码
│   ├── my_tsp.hpp / my_tsp.cpp      实例、距离矩阵、路径长度、最近邻、2-opt
│   ├── my_ga.hpp / my_ga.cpp        选择 / 交叉 / 变异算子
│   ├── my_tsp_main.cpp              命令行解析、实验主循环、CSV 输出
│   └── my_TSP_project.vcxproj       工程文件（C++20，v145 工具集）
├── results/                         实验输出（CSV，按配置分子目录）
├── run_tournament_k2_swap_seed_1,10.bat   批量跑 10 个随机种子的示例脚本
└── 排列组合几个策略.txt              参数速查表
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
| --save    | 保存方法                                     | whole        |

### 批量跑多个种子

`run_tournament_k2_swap_seed_1,10.bat` 演示了"同一配置换 10 个种子各跑一次"：

```bat
for /L %%i in (1,1,10) do (
    x64\Debug\my_TSP_project.exe --sel tournament --mut swap --seed %%i --save experiment
)
```

## 输出格式

每次运行在 `results/` 下生成仅一个文件。

**主结果** `<配置>_<种子>_<时间戳>.csv`：

```
tournament_OX_swap
pc= 0.90, pm= 0.30, seed= 10
tournament_k= 3

gen,best
-1,22984.4329
0,22214.6587
...
499,10275.6635
standard,6503.8798
```

- 前 4 行是元数据（配置、参数、锦标赛规模）；
- `gen = -1` 是初始种群的最优值；
- 最后一行 `standard` 是该实例上"最近邻 + 2-opt"基准的结果。

**绘图用** `<配置>_<种子>_<时间戳>_gragh.csv`：只有 `gen,best` 两列，可直接拖进
Excel / WPS 画收敛曲线。

多次重复实验（只改变种子）用<配置>\_<种子>.csv：只有策略参数和每次实验的最终结果

## 已跑的实验与初步结果

配置：`tournament`（k=3）+ `OX` + `swap`，`pc=0.9`、`pm=0.3`、`popS=100`、`gens=500`、
50 个城市（坐标 0–1000 均匀随机，seed=42 的实例），10 个随机种子各跑一次。

| 指标             | 数值               |
| -------------- | ---------------- |
| 最终最优（10 次）     | 14563.3 ± 5884.8 |
| 最好一次           | 9438.8           |
| 最差一次           | 23484.7          |
| 对照：最近邻 + 2-opt | 6503.9           |

两个观察：

1. **10 次里有 3 次严重早熟**，结果在 2.2–2.3 万，几乎等于随机回路的水平；
   另外 7 次落在 9.4–1.2 万。同一份代码、只换种子就出现这种分裂，说明该配置的多样性维持能力不够。
2. **把变异率调高后早熟缓解**：`pm=0.9` 与 `pm=1.0` 的两次对比实验（见 `results/` 里的文件名标注），
   结果是 `pm=1.0` 明显好于 `pm=0.9`，但还没有做成"同一批种子、同一组参数"的正式对照。

> 结论目前只能算"初步"：3 种选择 × 3 种变异的完整对比还没跑完，上面这些数字不要当成最终结果引用。

## 已知问题 / 待办

- [x] **结果路径是硬编码的绝对路径**（`E:\CodeTools\Project\...`），换台机器就跑不了，需改成相对路径 `results/`
- [ ] 早熟问题：`tournament` + `swap` 在部分种子上直接崩到随机解，需系统对比不同变异率 / 选择压力
- [ ] 缺 3 种选择 × 3 种变异的完整对照实验（每组 10 个种子）
- [ ] 结果只能人工画图，缺一个批量出图 / 汇总统计的脚本
- [ ] 结果文件名带中文与逗号（如 `seed_1,10`、`(严重早熟)`），跨平台与 URL 不友好，建议后续统一成
  `<sel>_<cross>_<mut>_seed<k>.csv`

## License

MIT
