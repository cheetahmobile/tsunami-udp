# Other — COMPILING.txt

# Other — COMPILING.txt 模块文档

## 功能概述

该模块提供关于如何编译和安装 Tsunami 网络工具的说明。它描述了从 CVS 获取源代码、配置构建环境、执行编译命令以及安装二进制文件的过程。

## 架构与内容结构

### 主要目标
- 提供完整的编译指南，包括跨平台支持（如大端系统）
- 描述如何使用 configure 和 make 工具链进行构建
- 解释可选的安装过程及将生成的二进制文件放置到系统路径中

### 编译流程组件
```
CVS获取 → 配置 → 构建 → 安装
```

### 关键子目录
- `./common`：包含通用 Makefile.am 文件，用于设置特定平台的编译选项
- `./server/tsunamid`：主服务器程序
- `./client/tsunami`：客户端程序
- `./rtserver/rttsunamid`：实时服务器程序
- `./rtclient/rttsunami`：实时客户端程序

## 使用方法

### 1. 获取源码
```bash
cvs -z3 -d:pserver:anonymous@tsunami-udp.cvs.sf.net:/cvsroot/tsunami-udp co -P tsunami-udp
```

### 2. 基本编译步骤
```bash
make
```

如果遇到 configure 错误，请先安装缺失依赖项：
```bash
./recompile.sh
```

### 3. 特定平台编译
#### 大端架构编译
编辑 `./common/Makefile.am` 并按照其中指示操作。

#### Solaris 编译
```bash
./configure CC=gcc CPPFLAGS=-Du_int64_t=uint64_t \
    -Du_int32_t=uint32_t -Du_int16_t=uint16_t \
    -Du_int8_t=uint8_t LIBS=-lsocket -lnsl -lrt
```

### 4. 单独编译子目录
```bash
cd client
make clean
make
```

> 注意：必须首先在根目录运行 `./configure` 和 `make` 来完成整个源树的编译后才能单独重新编译子目录。

### 5. 可选安装
```bash
sudo make install
```

安装位置通常为 `/usr/local/bin` 或类似目录。

## 可配置特性

### 实时服务器配置
在 `./rtserver/io.c` 中取消注释以下行以启用 3/4 模式传输：
```c
//#define MODE_34TH   
```

### 标准客户端配置
在 `./client/protocol.c` 中定义以下宏来启用线性排序块功能：
```c
#define RETX_REQBLOCK_SORTING
```

此功能可提高磁盘 I/O 性能，因为可以实现更连续的访问模式。