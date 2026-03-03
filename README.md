# OS-Project1 - 操作系统课程项目

操作系统课程实验代码，包含进程管理、进程通信、远程 Shell 和多线程排序等实现。

## 项目简介

本项目包含操作系统课程中的多个实验模块：

- **文件拷贝** - 使用 fork、exec、pipe 实现不同方式的文件拷贝
- **远程 Shell** - 基于 TCP 的远程命令行服务器
- **多线程排序** - 单线程与多线程归并排序性能对比

## 项目结构

```
OS-Project1/
├── Copy/                    # 文件拷贝实验
│   ├── ForkCopy.c          # fork + exec 方式拷贝
│   ├── MyCopy.c            # 基础文件拷贝
│   └── PipeCopy.c          # 管道进程通信拷贝
├── Shell/
│   └── shell.c             # 远程 Shell 服务器
├── Sort/                    # 归并排序实验
│   ├── MergesortSingle.c   # 单线程归并排序
│   └── MergesortMulti.c    # 多线程归并排序
└── README.md
```

## 模块说明

### 1. Copy - 文件拷贝实验

对比不同进程间通信方式的文件拷贝性能。

| 文件 | 实现方式 | 说明 |
|------|---------|------|
| `MyCopy.c` | 直接拷贝 | 基础文件拷贝，支持 `BUFFER_SIZE` 环境变量设置缓冲区大小 |
| `ForkCopy.c` | fork + exec | 创建子进程执行 MyCopy，统计执行时间 |
| `PipeCopy.c` | 管道通信 | 使用 pipe 在两个进程间传输数据，读写分离 |

#### 编译

```bash
cd Copy
gcc MyCopy.c -o MyCopy
gcc ForkCopy.c -o ForkCopy
gcc PipeCopy.c -o PipeCopy
```

#### 运行

```bash
# 基础拷贝
./MyCopy <源文件> <目标文件>

# 设置缓冲区大小（默认 4096 字节）
BUFFER_SIZE=8192 ./MyCopy <源文件> <目标文件>

# fork + exec 方式
./ForkCopy <源文件> <目标文件>

# 管道方式
./PipeCopy <源文件> <目标文件>
```

---

### 2. Shell - 远程 Shell 服务器

基于 TCP 的远程命令行服务器，支持多客户端连接。

#### 功能特性

- **多客户端支持** - 每个客户端独立进程处理
- **管道命令** - 支持 `cmd1 | cmd2` 形式的管道操作
- **内建命令** - 支持 `cd` 切换目录、`exit` 退出
- **彩色提示符** - 显示 `用户名@主机名:当前路径>`

#### 编译运行

```bash
cd Shell
gcc shell.c -o shell
./shell <端口号>
```

#### 客户端连接

```bash
nc <服务器IP> <端口号>
# 或
telnet <服务器IP> <端口号>
```

---

### 3. Sort - 归并排序性能对比

对比单线程与多线程归并排序的执行效率。

| 文件 | 实现方式 | 说明 |
|------|---------|------|
| `MergesortSingle.c` | 单线程 | 标准递归归并排序 |
| `MergesortMulti.c` | 多线程 | 使用 pthread 实现并行归并排序 |

#### 编译

```bash
cd Sort
gcc MergesortSingle.c -o MergesortSingle
gcc MergesortMulti.c -o MergesortMulti -lpthread -lm
```

#### 运行

```bash
# 单线程版本
./MergesortSingle
# 输入: 数组大小 n
# 输入: n 个整数

# 多线程版本（线程数必须是 2 的幂）
./MergesortMulti 4    # 使用 4 个线程
./MergesortMulti 8    # 使用 8 个线程
```

#### 输入格式

```
8                    # 数组大小 (8 ~ 10000000)
5 2 8 1 9 3 7 4      # 数组元素
```

## 环境要求

- Linux / Unix 操作系统
- GCC 编译器
- POSIX 线程库 (pthread)

## 作者

Man-in-the-Mirror-05
