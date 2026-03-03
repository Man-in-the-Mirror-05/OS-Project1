# OS-Project1 - 操作系统课程项目

操作系统课程实验代码，包含文件拷贝相关实现。

## 项目简介

本项目包含操作系统课程中文件拷贝机制的实现，包括：

- `ForkCopy` - 基于 fork 的文件拷贝实现
- `MyCopy` - 自定义文件拷贝实现
- `ForkCopy.c` - 源码文件

## 实验内容

### 文件拷贝实现

对比不同文件拷贝方式的性能和实现差异：

| 实现方式 | 说明 |
|---------|------|
| ForkCopy | 使用 fork 创建子进程进行文件拷贝 |
| MyCopy | 自定义实现的文件拷贝 |

## 项目结构

```
OS-Project1/
├── .vscode/            # VS Code 配置
├── Copy/               # 拷贝实现目录
│   ├── ForkCopy        # 编译后的可执行文件
│   ├── ForkCopy.c      # 源码
│   ├── MyCopy          # 编译后的可执行文件
│   └── ...
└── README.md           # 项目说明
```

## 编译与运行

### 编译

```bash
cd Copy
gcc ForkCopy.c -o ForkCopy
gcc MyCopy.c -o MyCopy
```

### 运行

```bash
./ForkCopy <源文件> <目标文件>
./MyCopy <源文件> <目标文件>
```

## 环境要求

- Linux 操作系统
- GCC 编译器
- POSIX 环境

## 作者

Man-in-the-Mirror-05
