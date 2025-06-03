# 贪吃蛇大作战

![贪吃蛇大作战LOGO](./picture/greedy_snake_battle_logo.jpg)

## 目录

- [项目概述](#项目概述)
- [项目许可证](#项目许可证)
- [游戏控制](#游戏控制)
- [游戏元素](#游戏元素)
- [游戏流程](#游戏流程)
- [离线模式](#离线模式)
- [Windows下需要](Windows下需要)
- [游戏接口](#游戏接口)
- [构建与安装](#构建与安装)
- [跨平台支持](#跨平台支持)
- [已知问题](#已知问题)
- [未来计划](#未来计划)
- [代码风格总结与贡献指南](#代码风格总结与贡献指南)

---
## 项目概述

这是一个用C语言实现的跨平台(Linux-Like，Windows需要Cygwin)贪吃蛇对战游戏，具有丰富的配置选项和完整的游戏功能。项目采用模块化设计，支持多种操作系统平台。   

作者: 张志宇  
许可证: MIT许可证: [LICENSE.txt](./LICENSE.txt)  
版本：5.0.2  
欢迎提交修改，建议和Issues。

## 项目许可证

### MIT许可证

**麻省理工学院许可证**（MIT License)。

### ​授权范围

- **​免费授权**​：任何人获得该软件及其相关文档文件（“软件”）的副本后，都可以免费使用。

- **​使用权限**​：被授权人可以不受限制地处理该软件，包括但不限于使用、复制、修改、合并、发布、分发、再授权以及销售软件副本。

### ​条件限制

- **​保留版权声明和许可声明**​：在软件的所有副本或重要部分中，必须包含上述版权声明和许可声明。

### ​免责声明

- **​无保证**​：该软件是“按原样”提供的，没有任何形式的保证，无论是明示还是暗示，包括但不限于对适销性、特定用途的适用性以及不侵权的保证。

- **​无责任**​：无论是在合同行为、侵权行为还是其他任何情况下，软件的作者或版权持有人都不对因软件或其使用而产生的任何索赔、损害或其他责任负责。​

### 查看许可证

MIT许可证：[LICENSE.txt](./LICENSE.txt)

## 游戏控制

| 按键 | 功能 |
| :--: | :--: |
| W | 向上移动 |
| A | 向左移动 |
| S | 向下移动 |
| D | 向右移动 |
| E | 立即退出游戏 |
| O | 退出当前对局 |
| J | 跳跃一格 |
| F | 持续跳跃 |
| P | 暂停当前对局 |

## 游戏元素

| 符号 | 代表元素 |
|------|----------|
| @ | 蛇头 |
| * | 蛇身 |
| # | 食物 |
| + | 墙 |
| $ | 胜利点 |

## 游戏流程

- 1. **客户端资源加载** (如果自行修改源代码后编译，可选择是否保留)  

- 2. **首次登录加载界面** (仅第一次运行游戏时显示，如果自行修改源代码后编译，可选择是否保留)  

- 3. **主菜单界面**
    按*上下键(或W,S)*移动**>**进行选择，按下*Tab*或*回车*表示确认选择
    | 功能 |
    | :--: |
    | 开始游戏 |
    | 游戏说明 |
    | 游戏设置 |
    | 退出游戏 |
- 4. **游戏设置界面**
    按*上下键(或W,S)*移动**>**进行选择，按下*Tab*或*回车*表示确认选择  
    进行设置时，下划线所在且高亮的数字为当前调整的一个数字，可以通过*上下键(或W,S)或直接输入数字调整*。可以通过*左右键(或A,D)*移动要调整的数字  
    可配置选项包括：
    - 蛇吃到自己是否退出
    - 蛇移动速度
    - 胜利点所需积分
    - 食物数量
    - 围墙数量
    - 是否开启系统小蛇
    - 游戏界面大小
    - 游戏背景颜色
    - 全部设置默认等

- 5. **游戏运行界面**

| 按键 | 功能 |
| :--: | :--: |
| W | 向上移动 |
| A | 向左移动 |
| S | 向下移动 |
| D | 向右移动 |
| E | 立即退出游戏 |
| O | 退出当前对局 |
| J | 跳跃一格 |
| F | 持续跳跃 |

- 6. **游戏结束**

---
## 离线模式

当配置文件无法打开时，游戏会自动进入离线模式：

- **特点**：
    - 不再读取配置文件
    - 使用内置默认配置运行游戏
    - 设置更改不会被保存
    - 游戏结束后配置恢复默认

- **注意事项**：
    - 离线模式下可以调整游戏设置，但不会保存
    - 不建议在离线模式下更改游戏设置
    - 离线模式一旦开启，在游戏结束前无法关闭

---
## Windows下需要

> 若您不是*开发人员*或*无查看游戏接口的需要*，则无需阅读以下内容。

Cygwin是一个在Windows上提供完整Linux-like环境的工具，它使你可以在Windows上使用大多数Linux命令和工具。以下是获取和安装Cygwin的详细步骤：

### 1. 下载Cygwin

访问Cygwin官方网站获取安装程序：
- 官方网站：[https://cygwin.com/](https://cygwin.com/)
- 直接下载链接：[https://cygwin.com/setup-x86_64.exe](https://cygwin.com/setup-x86_64.exe) (64位版本)

### 2. 安装Cygwin

#### 基本安装步骤

- 1. **运行安装程序**：
   - 双击下载的`setup-x86_64.exe`文件

- 2. **选择安装类型**：
   ```
   Install from Internet (推荐)
   Download Without Installing
   Install from Local Directory
   ```
   选择第一个选项"Install from Internet"

- 3. **选择安装目录**（默认通常是）：
   ```
   C:\cygwin64\
   ```
   注意：路径最好不要包含空格或中文

- 4. **选择本地包目录**（用于存储下载的包）：
   ```
   C:\Users\<你的用户名>\Downloads\cygwin-packages\
   ```

- 5. **选择连接方式**：
   - 如果你使用代理，在此处配置
   - 否则选择"Direct Connection"

#### 选择镜像站点

- 1. 从列表中选择一个镜像站点，例如：
   ```
   http://mirrors.163.com/cygwin/ (中国镜像)
   http://mirrors.kernel.org/sourceware/cygwin/
   ```

- 2. 点击"Next"继续

#### 选择要安装的包

- 1. 在搜索框中输入你需要的包，例如：
   - `gcc` (GNU编译器集合)
   - `make` 和 `cmake` (构建工具)
   - `gdb` (调试器)
   - `git` (GitHub仓库工具)

- 2. 点击每个包旁边的"Skip"将其变为版本号，表示要安装

- 3. 点击"Next"开始下载和安装

### 3. 安装后的配置

#### 环境变量设置

- 1. 将Cygwin的bin目录添加到系统PATH：
   ```
   C:\cygwin64\bin
   ```

- 2. 方法：
   - 右键"此电脑" → 属性 → 高级系统设置 → 环境变量
   - 在"系统变量"中找到Path，点击编辑
   - 添加新条目：`C:\cygwin64\bin`

#### 启动Cygwin

- 1. 通过开始菜单找到"Cygwin64 Terminal"并启动
- 2. 或者创建桌面快捷方式

### 4. 验证安装

在Cygwin终端中运行以下命令验证基本功能：

- 检查bash版本
```bash
bash --version
```

- 检查gcc是否安装
```bash
gcc --version
```

- 检查make工具
```bash
make --version
```

- 测试Linux命令
```bash
ls -l
```
```bash
uname -a
```

### 5. 安装额外包（后续）

如果需要安装更多包：

- 1. 重新运行`setup-x86_64.exe`
- 2. 选择"Install from Internet"
- 3. 在包选择界面搜索并选择新包
- 4. 完成安装

或者使用Cygwin的命令行安装工具`apt-cyg`：

```bash
lynx -source rawgit.com/transcode-open/apt-cyg/master/apt-cyg > apt-cyg # 首先安装apt-cyg
```
```bash
install apt-cyg /bin
```
```bash
apt-cyg install 包名 # 然后使用apt-cyg安装包
```

### 6. 使用Cygwin开发

现在你可以在Cygwin中使用`unistd.h`和其他POSIX功能了：

```c
#include <unistd.h>  // 现在可用

int main() {
    pid_t pid = fork();  // 支持fork()
    if (pid == 0) {
        printf("Child process\n");
    } else {
        printf("Parent process\n");
    }
    return 0;
}
```

编译命令：
```bash
gcc program.c -o program
./program
```

### 注意事项

- 1. Cygwin的性能略低于原生Linux系统
- 2. 某些Linux特有功能可能不完全支持
- 3. 对于高性能需求，考虑使用WSL2
- 4. 文件路径在Windows和Cygwin间转换：
   - Windows路径：`C:\path\to\file`
   - Cygwin路径：`/cygdrive/c/path/to/file`

---
## 游戏接口

游戏提供标准化的外部接口`GreedySnakeBattleGameExternalInterface`函数，定义在 [GreedySnakeBattleGameExternalInterface.h](./source/Include/GreedySnakeBattleGameExternalInterface.h) 中。

### 连接选项

| 类型 | 库名称 | 连接指令 |
| :--: | :----: | :------: |
| 静态库 | libgsnakebg.a | `gcc ... -lgsnakebg` |
| 动态库 | libgsnakebg.so | `gcc ... -lgsnakebg` |

### 函数原型

```c
#include <GreedySnakeBattleGameExternalInterface.h>

    int GreedySnakeBattleGameExternalInterface(StartupMode mode);
```

### 参数

`mode`: 贪吃蛇大作战游戏运行是否阻塞调用该函数的进程。 
| 枚举 | 行为 |
| :----: | :-----: |
| `SNAKE_BLOCK` | 阻塞运行 |
| `SNAKE_NONBLOCK` | 非阻塞运行 |

### 返回值

| 返回值 | 描述 |
| :----: | :--: |
| -2 | 参数错误 |
| -1 | 系统调用失败 |
| 0 | 阻塞模式下游戏结束 |
| 1 | 非阻塞模式下游戏启动成功 |

### 使用示例

```c
#include "GreedySnakeBattleGameExternalInterface.h"

int main() {
    int ret = GreedySnakeBattleGameExternalInterface(SNAKE_BLOCK);
    
    switch (ret) {
        case -2:
            printf("参数错误!\n");
            break;
        case -1:
            printf("游戏启动失败!\n");
            break;
        case 0:
            printf("游戏结束.\n");
            break;
        case 1:
            printf("游戏已在后台运行.\n");
            break;
    }
    
    return 0;
}
```

### 代码标准

| 项目 | 标准 |
| :--: | :--: |
| 编程语言 | C程序语言 - C99(ISO/IEC 9899:1999) |
| Unix-like系统 | POSIX.1-2008(Portable Operating System Interface of UNIX) |

---
## 构建与安装

### 构建要求

- CMake 3.10+
- C编译器 (GCC/Clang/MSVC)

### 构建步骤

- 1. 创建并进入build目录(保持项目根目录干净)
```bash
mkdir build # 创建build目录
```
```bash
cd build    # 进入build目录
```

- 2. 执行CMake
```bash
cmake ..    # 执行CMake
```

- 3. 执行Make编译源文件  
     ```bash
     make   # 执行Make编译源文件
     ```
     生成GreedySnakeBattle(.exe)可执行文件，libgsnakebg.so/.dll动态库和libgsnakebg.a静态库。
    
### 执行Make进行安装/卸载

#### 安装

```bash
sudo make install  # 安装(需要管理员权限)(Termux无需管理员权限)
```

#### 卸载

```bash
sudo make uninstall       # 卸载(需要管理员权限)(Termux无需管理员权限)
```

### 安装内容

- 1. **可执行文件**：`GreedySnakeBattle` (Windows下为 `GreedySnakeBattle.exe`)
- 2. **库文件**：
   - 静态库：`libgsnakebg.a`
   - 动态库：`libgsnakebg.so` (Windows下为 `libgsnakebg.dll`)
- 3. **头文件**：`GSnakeBInclude/` 目录下的所有头文件

---
## 跨平台支持

游戏支持以下平台：

- 1. **Windows**:
    - 需要Cygwin
    - 自动创建桌面快捷方式
    - 设置应用图标

- 2. **Linux**:
    - 标准Unix安装路径
    - 创建.desktop桌面快捷方式
    - 使用termios实现终端控制

- 3. **Termux(Android)**:
    - 特殊安装路径适配
    - 针对移动终端优化

- 4. **MacOS**:
    - Unix标准安装路径
    - 使用termios实现终端控制

---
## 已知问题

1. **性能问题**：
    - 当蛇身足够大时，游戏可能会出现卡死现象

2. **功能限制**：
    - 障碍蛇无法自动避开用户蛇和墙壁
    - 当用户蛇吃到障碍蛇身体时，逻辑处理不够精确

详细问题列表见 [GreedySnakeBattleGameExternalInterface.c](./source/GreedySnakeBattleGameExternalInterface.c) 中的BUGs部分

## 未来计划

1. **新功能**：
    - 设置传送门
    - 无限食物模式

2. **改进**：
    - 优化障碍蛇AI
    - 改进蛇身绘制逻辑

## 代码风格总结与贡献指南

基于该项目，总结了以下**注释风格**、**命名规则**和**代码风格**，供贡献者参考。  

### 1. 注释风格（Javadoc 风格）

#### 规则：

- 1.**文件注释**（文件开头）：  
```c
/**
 * @file GreedySnakeBattleGameExternalInterface.c
 * @brief This file implements the GreedySnakeBattleGameExternalInterface function.
 * @author Zhang Zhiyu
 */
```

- 2.**函数注释**（详细说明函数作用、参数、返回值）：  
```c
/**
 * @brief Initialize terminal settings
 * 
 * Save the original terminal mode and enable ANSI escape code support.
 */
static void initTerminalSettings() {
    // ...
}
```

- 3.**结构体/枚举注释**：  
```c
/**
 * @struct GameAllRunningData
 * @brief Store all data when the game is running.
 */
typedef struct {
    int score;  /**< Current score */
    // ...
} GameAllRunningData;
```

- 4. **避免**：
    - 单行注释 `//` 仅用于临时调试，正式代码应使用 `/** */` 或 `/* */`。  
    - 无意义的注释，如 `// This is a variable.`。  

#### 贡献者注意事项：
- **修改函数** 时，更新对应的说明。  
- **关键逻辑** 应补充`@note`或`@attention`说明特殊情况。  

---
### 2.命名规则（Qt风格）

采用**Qt的命名风格**：  

#### 规则：
| 类型 | 命名风格 | 示例 |
| :--: | :------: | :--: |
| **函数** | 除特别要求的函数组(蛇形命名法+小驼峰命名法:`lowerCamelCase_snake_case`外，其他函数都采用小驼峰命名法：`lowerCamelCase` | `setConfig_isEnableObs`，`foodInit` |
| **变量** | 小驼峰命名法:`lowerCamelCase` | `gameScore` |
| **结构体** | 结构体名: 大驼峰命名法: `UpperCamelCase`。成员变量: 小驼峰命名法: `lowerCamelCase` | `GameConfig`，`isEnableObs` |
| **宏/枚举/全局常量** | 大驼峰命名法: `UPPER_CASE` | `SNAKE_BLOCK`, `ENABLE_ECHO_INPUT` |
| **全局变量** | 小驼峰命名法：`lowerCamelCase` | `isConfigFileOpenFail` |
| **类型定义** | `PascalCase` + `_t`（当该类型为typedef已有类型时必须要加该后缀） | `GameAllRunningData`, `muint_t` |

#### 贡献者注意事项：
- **新变量/函数/...** 采用上述命名方式。
- **避免**使用`匈牙利命名法`（如`bIsRunning`, `iCount`）。  
- **宏定义**必须全部大写，如 `#define MAX_SNAKE_LENGTH 100`。  

---
### 3. 代码风格（Qt风格）

#### 规则：

- 1.**缩进**：
    - **4空格缩进**  
    - 函数体、`if/for/while` 块必须缩进  

- 2.**大括号`{}`**：
    - **K&R 风格**（左大括号不换行）：  
    ```c
    if (condition) {
        // code
    }
    ```

- 3.**空格**：
    - 运算符两侧加空格：  
        ```c
        int sum = a + b;
        if (score > 100) { ... }
        ```
    - 函数参数逗号后加空格：  
        ```c
        void foo(int a, int b, int c);
        ```

- 4.**避免**：
    - 一行多语句，如 `a=1; b=2;`。  
    - 过长的行（建议 80~120 字符换行）。  

#### 贡献者注意事项：
- **新增代码** 必须遵循现有缩进和括号风格。  
- **复杂逻辑** 应拆分成小函数，避免超长函数。
