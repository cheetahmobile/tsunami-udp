# Other — common

# Other — common 模块文档

## 功能概述

common 模块是一个基础工具库，提供了一系列通用的实用函数和数据结构。该模块主要包含 MD5 哈希计算、错误处理以及一些底层的通用功能实现。它为整个 Tsunami 软件包提供了必要的基础设施支持。

## 架构设计

### 核心组件

```
libtsunami_common.a
├── md5.c        # MD5 哈希算法实现
├── common.c     # 通用辅助函数
└── error.c      # 错误处理机制
```

### 文件组成

- `md5.c`: 实现了 MD5 哈希算法，用于生成数据摘要
- `common.c`: 提供各种通用辅助函数，如内存操作等
- `error.c`: 实现错误处理机制，包括错误码管理和报告

### 编译配置

该模块通过 `Makefile.am` 配置编译：
```makefile
AM_CPPFLAGS = -I$(top_srcdir)/include
noinst_LIBRARIES = libtsunami_common.a
libtsunami_common_a_SOURCES = md5.c common.c error.c
```

## 使用方法

### 编译集成

此模块被构建为静态库 `libtsunami_common.a`，需要在链接时指定：

```makefile
# 在其他模块的 Makefile.am 中添加
lib_LTLIBRARIES = libtsunami_example.la
libtsunami_example_la_SOURCES = example.c
libtsunami_example_la_LIBADD = $(top_builddir)/common/libtsunami_common.a
```

### API 接口

#### MD5 相关函数
- `MD5_Init()`: 初始化 MD5 上下文
- `MD5_Update()`: 更新 MD5 数据
- `MD5_Final()`: 完成 MD5 计算并获取结果

#### 错误处理相关
- `set_error()`: 设置错误状态
- `get_error()`: 获取当前错误信息

#### 通用辅助函数
- `safe_malloc()`: 安全分配内存
- `safe_free()`: 安全释放内存

### 依赖关系

该模块不直接调用外部函数，也不被其他模块调用。它作为独立的基础库存在，为上层模块提供通用服务。

## 注意事项

1. **字节序处理**: 
   - 对于大端平台（如 PlayStation3），需要在 configure 之前定义宏 `-DARCH_IS_BIG_ENDIAN`
   
2. **头文件包含**:
   - 所有使用该模块的源文件都需要包含 `common.h` 头文件

3. **编译选项**:
   - 模块编译时会自动包含 `$(top_srcdir)/include` 路径
   - 不需要额外的链接标志或库路径设置

## 开发建议

1. **维护原则**: 保持代码简洁、高效，避免过度复杂化
2. **测试策略**: 应当针对所有公共接口编写单元测试
3. **文档规范**: 新增函数应当附带清晰的注释说明其用途和参数要求