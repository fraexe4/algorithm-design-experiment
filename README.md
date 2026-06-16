# 算法设计与分析实验

> Spring 2026 算法设计与分析实验代码仓库
> 包含两个子实验：排序算法对比 + 0-1 背包多算法对比

## 目录结构

```
algorithm-design-experiment/
├── sort/                  # 实验一：排序算法对比
│   ├── sort.cpp           # 冒泡 / 归并 / 快速排序
│   ├── data/              # 测试样本 (CSV)
│   └── README.md
└── 01Backpack/            # 实验二：0-1 背包
    ├── 01backpack.cpp     # 暴力 / DP / 贪心 / 回溯
    ├── data/              # 物品样本 (CSV)
    └── README.md
```

## 编译与运行

两个子项目均为**纯 C++ 实现**，无第三方依赖：

```bash
# 排序
g++ -O2 -std=c++11 sort/sort.cpp -o sort/sort
./sort/sort

# 背包
g++ -O2 -std=c++11 01Backpack/01backpack.cpp -o 01Backpack/01backpack
./01Backpack/01backpack
```

也支持直接用 Code::Blocks 打开对应 `.cbp` 工程文件。

## 实验内容

### 实验一：排序算法对比
- **算法**：冒泡排序 O(n²) / 归并排序 O(n log n) / 快速排序 O(n log n)
- **任务**：
  - 任务 1：固定 n=10000，2 组种子，对比时间
  - 任务 2：n = 10, 100, 1000, 10000, 50000，对比时间
  - 任务 3：不同分布（顺序 / 逆序 / 随机）下对比时间
- **数据**：`sort/data/sort_*.csv`

### 实验二：0-1 背包多算法对比
- **算法**：暴力枚举 O(2ⁿ) / 动态规划 O(nC) / 贪心 / 回溯+上界剪枝
- **规模**：
  - 小规模：n = 10, 15, 20, 24，四种算法全跑
  - 大规模：n = 1000 ~ 20000，只跑 DP + 贪心
- **容量**：C = 10000 / 100000 / 1000000
- **数据**：`01Backpack/data/ks_*.csv`
