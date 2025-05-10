# 贪吃蛇大作战

![贪吃蛇大作战LOGO](./picture/greedy_snake_battle_logo.jpg)

## 目录

- [项目概述](#项目概述)
- [项目许可证](#项目许可证)
- [游戏特点](#游戏特点)
- [游戏控制](#游戏控制)
- [游戏元素](#游戏元素)
- [游戏流程](#游戏流程)
- [离线模式](#离线模式)
- [签名验证](#签名验证)
- [Windows下游戏需要](Windows下游戏需要)
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
版本：3.0.1  
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

---
## 游戏特点

- 经典贪吃蛇游戏玩法
- 支持自定义游戏设置
- 跨平台支持（Windows(需要Cygwin)/Linux/MacOS/Termux)
- 提供阻塞和非阻塞两种运行模式
- 完善的API接口
- 详细的文档支持

## 游戏控制

![游戏对局中](./picture/game_running.jpg)  
| 按键 | 功能 |
| :--: | :--: |
| W | 向上移动 |
| A | 向左移动 |
| S | 向下移动 |
| D | 向右移动 |
| E | 立即退出游戏 |
| O | 退出当前对局 |

## 游戏元素

![游戏介绍](./picture/introduction.jpg)
| 符号 | 代表元素 |
|------|----------|
| @ | 蛇头 |
| * | 蛇身 |
| # | 食物 |
| + | 墙 |
| $ | 胜利点 |

## 游戏流程

- 1. **客户端资源加载**（如果自行修改源代码后编译，可选择是否保留  
   ![客户端资源加载界面](./picture/client_resource_loading.jpg)  
- 2. **首次登录加载界面**（仅第一次运行游戏时显示，如果自行修改源代码后编译，可选择是否保留  
   ![首次加载界面](./picture/first_login_loading.jpg)  

- 3. **主菜单界面**

   ![主菜单界面](./picture/menu.jpg)

   | 选项 | 功能 |
   | :--: | :--: |
   | 1 | 开始游戏 |
   | 2 | 游戏说明 |
   | 3 | 游戏设置 |
   | 4 | 退出游戏 |

- 4. **游戏设置界面**

   ![设置界面](./picture/set_game.jpg)

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

   ![游戏运行界面](./picture/game_running.jpg)

- 6. **游戏结束**（胜利或失败）

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
## 签名验证

> 若您不是*开发人员*或*无查看游戏接口的需要*，则无需阅读以下内容。

### 预备

需要：GPG：GPG官网：[https://gnupg.org](https://gnupg.org)  
Windows版本(GPG4Win) [https://gpg4win.org](https://gpg4win.org)  

#### **安装GPG**

##### **Windows**
1. 下载安装 [GPG4Win](https://gpg4win.org/download.html)。
2. 安装时勾选**"GPG (GnuPG) Command Line"** 以确保 `gpg.exe` 可用。

##### **Linux/MacOS**

- Linux
    ```bash
    sudo (安装包管理器) install gnupg
    ```
    把`(安装包管理器)`替换为系统的安装包管理器(如，Ubuntu/Debian替换为apt)
- MacOS
    ```shell
    brew install gnupg
    ```

### 1.解压签名文件

- 1.Linux/MacOS操作系统
    ```bash
    unzip signatures.zip        # 直接解压到signatures目录，无需单独创建目录
    ```
- 2.Windows操作系统
    使用Windows内置功能。  
    - 1. 打开文件资源管理器，找到项目根目录下的`signatures.zip`文件。  
    - 2. 双击`signatures.zip`，系统会自动用“*文件资源管理器*”打开。  
    - 3. 点击顶部菜单的“*解压全部*”，选择当前目录后点击“解压”。  

### 2.导入公钥(位于项目根目录下)：

```bash
gpg --import ./signatures/PublicKey/GreedySnakeBattle_PublicKey.asc
```

### 3.验证签名：

- 单个文件验证(以LICENSE.txt文件为例)
    ```bash
    gpg --verify signatures/LICENSE.txt.asc LICENSE.txt
    ```

- 批量验证
    - 源文件验证(Unix-Like)
        ```bash
        find source/ -type f \( -name "*.c" -o -name "*.h" \) -exec sh -c 'gpg --verify signatures/$(basename {}).asc {}' \;
        ```
    - LICENSE.txt许可证文档验证
        ```bash
        gpg --verify signatures/LICENSE.txt.asc LICENSE.txt
        ```

- 确认输出
    1. 包含`Good signature from "Zhang Zhiyu (A C And Cpp Development Designer) <2585689367@qq.com>`表示源文件和签名未被修改。 b
        ```txt
        gpg: Signature made 20XX-XX-XX XX:XX:XX +0800 CST
        gpg:                using RSA key CAD77FD957132D24B9B75D1AFFAB5EB03E8460D0
        gpg: Good signature from "Zhang Zhiyu (A C And Cpp Development Designer) <2585689367@qq.com>" [ultimate]
        ```
    
    2. 包含`BAD signature from "Zhang Zhiyu (A C And Cpp Development Designer) <2585689367@qq.com>`表示**源文件和签名被篡改**。  
        **此时不建议再使用这些文件，因为这些文件可能被篡改过，可能存在未知风险！！！**  
        ```txt
        gpg: Signature made 20XX-XX-XX XX:XX:XX +0800 CST
        gpg:                using RSA key CAD77FD957132D24B9B75D1AFFAB5EB03E8460D0
        gpg: BAD signature from "Zhang Zhiyu (A C And Cpp Development Designer) <2585689367@qq.com>" [ultimate]
        ```

---
### 自动化检查脚本

#### Windows

将以下脚本保存为 `verify_all.ps1`，放在项目根目录运行：
```powershell
# 导入公钥（如果尚未导入）
$publicKey = ".\signatures\PublicKey\GreedySnakeBattle_PublicKey.asc"
gpg --import $publicKey

# 验证源码文件
Get-ChildItem -Path .\source -Recurse -Include *.c, *.h | ForEach-Object {
    $sigFile = ".\signatures\" + $_.Name + ".asc"
    if (Test-Path $sigFile) {
        Write-Host "验证文件: $($_.FullName)"
        gpg --verify $sigFile $_.FullName
        
        # 错误检查
        if ($LASTEXITCODE -ne 0) {
            Write-Host "错误: $($_.FullName) 验证失败！" -ForegroundColor Red
            exit 1
        }
    }
}

# 验证 LICENSE.txt
gpg --verify .\signatures\LICENSE.txt.asc .\LICENSE.txt

Write-Host "验证完成！检查是否有 'BAD signature' 输出。" -ForegroundColor Green
```

以管理员身份运行PowerShell，然后执行：
```powershell
Set-ExecutionPolicy RemoteSigned -Scope Process  # 允许运行脚本
```
```powershell
.\verify_all.ps1
```

#### Linux/MacOS

##### **1. 脚本内容**  

将以下代码保存为 `verify_signatures.sh`，并放在项目根目录：  
```bash
#!/bin/bash

# ------------------------------------------
# GreedySnakeBattle 签名验证脚本 (Linux/MacOS)
# 功能：自动验证所有 .c/.h/README.md/LICENSE.txt 的 GPG 签名
# 用法：./verify_signatures.sh
# ------------------------------------------

# 检查是否安装了 GPG
if ! command -v gpg &> /dev/null; then
    echo "❌ 错误：GPG 未安装！请先安装："
    echo "  - Debian/Ubuntu: sudo apt install gnupg"
    echo "  - MacOS: brew install gnupg"
    exit 1
fi

# 定义签名目录和公钥路径
SIGNATURES_DIR="./signatures"
PUBLIC_KEY="$SIGNATURES_DIR/PublicKey/GreedySnakeBattle_PublicKey.asc"

# 检查签名目录是否存在
if [ ! -d "$SIGNATURES_DIR" ]; then
    echo "❌ 错误：签名目录 $SIGNATURES_DIR 不存在！"
    exit 1
fi

# 检查公钥是否存在
if [ ! -f "$PUBLIC_KEY" ]; then
    echo "❌ 错误：公钥文件 $PUBLIC_KEY 未找到！"
    exit 1
fi

# 导入公钥
echo "🔑 导入公钥..."
gpg --import "$PUBLIC_KEY" 2> /dev/null
if [ $? -ne 0 ]; then
    echo "❌ 错误：公钥导入失败！"
    exit 1
fi

# 验证所有源码文件（.c 和 .h）
echo "🔍 开始验证源码文件..."
FAILED=0
for file in $(find ./source -type f \( -name "*.c" -o -name "*.h" \)); do
    sig_file="$SIGNATURES_DIR/$(basename $file).asc"
    if [ -f "$sig_file" ]; then
        echo "📄 验证: $file"
        gpg --verify "$sig_file" "$file" 2> /dev/null
        if [ $? -ne 0 ]; then
            echo "❌ 验证失败: $file"
            FAILED=1
        fi
    else
        echo "⚠️ 警告: 未找到签名文件: $sig_file"
    fi
done

# 验证 LICENSE.txt
echo "📜 验证文档文件..."
for file in "LICENSE.txt"; do
    sig_file="$SIGNATURES_DIR/$file.asc"
    if [ -f "$sig_file" ]; then
        echo "📄 验证: $file"
        gpg --verify "$sig_file" "$file" 2> /dev/null
        if [ $? -ne 0 ]; then
            echo "❌ 验证失败: $file"
            FAILED=1
        fi
    else
        echo "⚠️ 警告: 未找到签名文件: $sig_file"
    fi
done

# 最终结果
if [ $FAILED -eq 0 ]; then
    echo "✅ 所有文件验证通过！"
else
    echo "❌ 部分文件验证失败，请检查日志！"
    exit 1
fi
```

##### **2. 使用方法**

1. **赋予脚本执行权限**：  
   ```bash
   chmod +x verify_signatures.sh
   ```

2. **运行脚本**：  
   ```bash
   ./verify_signatures.sh
   ```

3. **预期输出**：  
   - 自动检查依赖：
     如果未安装 GPG，会提示用户安装  
   - 批量验证：
    ;自动遍历 `./source/` 下的所有 `.c` 和 `.h` 文件  
   - 如果所有文件验证通过：  
     ```txt
     ✅ 所有文件验证通过！
     ```
   - 如果某些文件验证失败：  
     ```txt
     ❌ 验证失败: ./source/main.c
     ❌ 部分文件验证失败，请检查日志！
     ```
   - 如果签名文件缺失：
     会提示警告（`⚠️`）。  

---
## Windows下游戏需要

Cygwin是一个在Windows上提供完整Linux-like环境的工具，它允许你在Windows上使用大多数Linux命令和工具。以下是获取和安装Cygwin的详细步骤：

### 1. 下载Cygwin

访问Cygwin官方网站获取安装程序：
- 官方网站：[https://cygwin.com/](https://cygwin.com/)
- 直接下载链接：[https://cygwin.com/setup-x86_64.exe](https://cygwin.com/setup-x86_64.exe) (64位版本)

### 2. 安装Cygwin

#### 基本安装步骤

1. **运行安装程序**：
   - 双击下载的`setup-x86_64.exe`文件

2. **选择安装类型**：
   ```
   Install from Internet (推荐)
   Download Without Installing
   Install from Local Directory
   ```
   选择第一个选项"Install from Internet"

3. **选择安装目录**（默认通常是）：
   ```
   C:\cygwin64\
   ```
   注意：路径最好不要包含空格或中文

4. **选择本地包目录**（用于存储下载的包）：
   ```
   C:\Users\<你的用户名>\Downloads\cygwin-packages\
   ```

5. **选择连接方式**：
   - 如果你使用代理，在此处配置
   - 否则选择"Direct Connection"

#### 选择镜像站点

1. 从列表中选择一个镜像站点，例如：
   ```
   http://mirrors.163.com/cygwin/ (中国镜像)
   http://mirrors.kernel.org/sourceware/cygwin/
   ```

2. 点击"Next"继续

#### 选择要安装的包

1. 在搜索框中输入你需要的包，例如：
   - `gcc` (GNU编译器集合)
   - `make` (构建工具)
   - `gdb` (调试器)
   - `vim` 或 `emacs` (编辑器)
   - `openssh` (SSH客户端/服务器)
   - `git` (版本控制)

2. 点击每个包旁边的"Skip"将其变为版本号，表示要安装

3. **重要开发包**：
   - `gcc-core`：C编译器
   - `gcc-g++`：C++编译器
   - `make`：构建工具
   - `gdb`：调试器
   - `libncurses-devel`：终端处理库
   - `git`：版本控制

4. 点击"Next"开始下载和安装

### 3. 安装后的配置

#### 环境变量设置

1. 将Cygwin的bin目录添加到系统PATH：
   ```
   C:\cygwin64\bin
   ```

2. 方法：
   - 右键"此电脑" → 属性 → 高级系统设置 → 环境变量
   - 在"系统变量"中找到Path，点击编辑
   - 添加新条目：`C:\cygwin64\bin`

#### 启动Cygwin

1. 通过开始菜单找到"Cygwin64 Terminal"并启动
2. 或者创建桌面快捷方式

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

1. 重新运行`setup-x86_64.exe`
2. 选择"Install from Internet"
3. 在包选择界面搜索并选择新包
4. 完成安装

或者使用Cygwin的命令行安装工具`apt-cyg`：

```bash
# 首先安装apt-cyg
lynx -source rawgit.com/transcode-open/apt-cyg/master/apt-cyg > apt-cyg
install apt-cyg /bin

# 然后使用apt-cyg安装包
apt-cyg install nano
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

1. Cygwin的性能略低于原生Linux系统
2. 某些Linux特有功能可能不完全支持
3. 对于高性能需求，考虑使用WSL2
4. 文件路径在Windows和Cygwin间转换：
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

    int GreedySnakeBattleGameExternalInterface(int isBlockRunning);
```

### 参数

`isBlockRunning`: 贪吃蛇大作战游戏运行是否阻塞调用该函数的进程。 
| 宏(实为全局常量) | 行为 |
| :----: | :-----: |
| SNAKE_BLOCK | 阻塞运行 |
| SNAKE_UNBLOCK | 非阻塞运行 |

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
- Doxygen (用于文档生成)(若没有也可以)

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

- 3. 执行Make
    - 1. 执行Make编译源文件  
         ```bash
         make                        # 执行Make编译源文件
         ```
         生成GreedySnakeBattle(.exe)可执行文件，libgsnakebg.so动态库和libgsnakebg.a静态库。
    
    - 2. 执行Make生成API文档
         - 1. 有Doxygen命令程序
              ```bash
              make API_documents     # 生成API文档
              ```
         - 2. 无Doxygen命令程序
              - 1.解压API文档文件.  
                  - 1.Linux/MacOS操作系统
                      ```bash
                      unzip ../pre_generated_API_documents.zip      # 直接解压到./pre_generated_API_documents目录，无需自行创建目录或移动目录
                      ```
                  - 2.Windows操作系统
                      使用Windows内置功能。  
                      - 1. 打开*文件资源管理器*，找到项目根目录下的`pre_generated_API_document.zip`文件。
                      - 2. 双击`pre_generated_API_documents.zip`，系统会自动用*“文件资源管理器”*打开。
                      - 3. 点击顶部菜单的*“解压全部”*，选择`build`目录后点击*“解压”*。
              - 2.重命名
                  ```bash
                  mv pre_generated_API_documents API_documents          # 把 pre_generated_API_documents 重命名为 API_documents
                  ```

### 执行Make进行安装/卸载

#### 安装

```bash
sudo make install         # 安装(需要管理员权限)(Termux无需管理员权限)
```

#### 卸载

```bash
sudo make uninstall       # 卸载(需要管理员权限)(Termux无需管理员权限)
```

### 安装内容

1. **可执行文件**：`GreedySnakeBattle` (Windows下为 `GreedySnakeBattle.exe`)
2. **库文件**：
   - 静态库：`libgsnakebg.a`
   - 动态库：`libgsnakebg.so` (Windows下为 `libgsnakebg.dll`)
3. **头文件**：`Include/` 目录下的所有头文件
4. **API文档**：HTML格式的API文档

---
## 跨平台支持

游戏支持以下平台：

1. **Windows**:
    - 需要Cygwin
    - 自动创建桌面快捷方式
    - 设置应用图标
    - 使用Win32 API实现终端控制

2. **Linux**:
    - 标准Unix安装路径
    - 创建.desktop桌面快捷方式
    - 使用termios实现终端控制

3. **Termux(Android)**:
    - 特殊安装路径适配
    - 针对移动终端优化

4. **MacOS**:
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

### **1. 注释风格（Javadoc 风格）**  
代码主要使用**Doxygen/Javadoc风格的注释**，适用于生成API文档。  

#### **规则**：

✅ **文件注释**（文件开头）：  
```c
/**
 * @file GreedySnakeBattleGameExternalInterface.c
 * @brief 该文件实现了 @ref GreedySnakeBattleGameExternalInterface 功能
 * @author Zhang Zhiyu
 */
```

✅ **函数注释**（详细说明函数作用、参数、返回值）：  
```c
/**
 * @brief 初始化终端设置
 * @ingroup OSAdapt
 * 
 * 保存原始终端模式并启用 ANSI 转义码支持。
 */
static void windowsInitConsole() {
    // ...
}
```

✅ **结构体/枚举注释**：  
```c
/**
 * @struct GameAllRunningData
 * @brief 存储游戏运行时的所有数据
 */
typedef struct {
    int score;  /**< 当前分数 */
    // ...
} GameAllRunningData;
```

❌ **避免**：
- 单行注释 `//` 仅用于临时调试，正式代码应使用 `/** */` 或 `/* */`。  
- 无意义的注释，如 `// 这是一个变量`。  

#### **贡献者注意事项**：
- **新增函数/文件** 必须包含完整的Doxygen注释。  
- **修改函数** 时，更新对应的`@brief`和`@param`等说明。  
- **关键逻辑** 应补充`@note`或`@attention`说明特殊情况。  

---
### **2.命名规则（Qt风格）**

采用**Qt的命名风格**，但略有调整：  

#### **规则**：
| 类型 | 命名风格 | 示例 |
| :--: | :------: | :--: |
| **函数** | 除特别要求的函数组(蛇形命名法+小驼峰命名法:`lowerCamelCase_snake_case`外，其他函数都采用小驼峰命名法：`lowerCamelCase` | `setConfig_isEnableObs`，`foodInit` |
| **变量** | 小驼峰命名法:`lowerCamelCase` | `gameScore` |
| **结构体** | 结构体名: 大驼峰命名法: `UpperCamelCase`。成员变量: 小驼峰命名法: `lowerCamelCase` | `GameConfig`，`isEnableObs` |
| **宏/枚举/全局常量** | 大驼峰命名法: `UPPER_CASE` | `SNAKE_BLOCK`, `ENABLE_ECHO_INPUT` |
| **全局变量** | 小驼峰命名法：`lowerCamelCase` | `isConfigFileOpenFail` |
| **类型定义** | `PascalCase` + `_t`（当该类型为typedef已有类型时必须要加该后缀） | `GameAllRunningData`, `muint_t` |

#### **贡献者注意事项**：
- **新变量/函数/...** 采用上述命名方式。
- **避免**使用`匈牙利命名法`（如`bIsRunning`, `iCount`）。  
- **宏定义**必须全部大写，如 `#define MAX_SNAKE_LENGTH 100`。  

---
### **3. 代码风格（Qt风格）**  
#### **规则**：
✅ **缩进**：
- **4 空格缩进**（非Tab）。  
- 函数体、`if/for/while` 块必须缩进。  

✅ **大括号`{}`**：
- **K&R 风格**（左大括号不换行）：  
```c
if ( condition ) {
    // code
}
```

✅ **空格**：
- 运算符两侧加空格：  
```c
int sum = a + b;
if ( score > 100 ) { ... }
```
- 函数参数逗号后加空格：  
```c
void foo(int a, int b, int c);
```

❌ **避免**：
- 一行多语句，如 `a=1; b=2;`。  
- 过长的行（建议 80~120 字符换行）。  

#### **贡献者注意事项**：
- **新增代码** 必须遵循现有缩进和括号风格。  
- **修改代码** 时保持风格一致，不要混用 `if (x){` 和 `if (x) {`。  
- **复杂逻辑** 应拆分成小函数，避免超长函数。  

---
### **总结：贡献者应遵循的规则**  
| 项目 | 规则 |
| :--: | :--: |
| **注释** | Doxygen&Javadoc风格，函数、文件、关键变量必须注释 |
| **命名** | 如上 |
| **代码风格** | 4空格缩进，K&R大括号，指针`*`靠近变量 |

### **如何检查？**  
1. **Clang-Format**（可配置 `.clang-format` 文件）。  
2. **Doxygen**生成文档，检查注释完整性。  
3. **代码审查**时人工核对命名和风格。  

希望贡献者能保持代码风格统一，提高可维护性！
