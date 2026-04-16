# Other — configure.ac

# configure.ac 模块文档

## 功能概述

`configure.ac` 是一个用于生成 `configure` 脚本的 Autoconf 配置文件。该脚本在编译时运行，检测系统环境、设置编译选项，并为构建过程生成合适的 Makefile 和配置头文件（如 `config.h`）。此模块主要用于确保 Tsunami 软件包能够在不同平台上正确编译和链接。

## 架构与关键组件

### 初始化阶段

```bash
AC_INIT([tsunami], [1.1b43])
echo "Configuring Tsunami version AC_PACKAGE_VERSION"
```

初始化项目名称和版本号，并输出当前配置信息。

```bash
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h:config.in)
```

使用 Automake 初始化并指定生成 config.h 的模板文件。

### 清理规则定义

```bash
CLEANFILES="*~"
AC_SUBST(CLEANFILES)
```

定义自动清理的文件类型，包括 Emacs 临时文件（以 ~ 结尾）。

### 编译器检查

```bash
AC_CANONICAL_HOST
CFLAGS_save="$CFLAGS"
AC_PROG_CC
if test "$GCC" != "yes"; then
    AC_MSG_WARN([*** WARNING! *** You are not using the GNU C compiler.])
    ...
fi
AC_PROG_RANLIB
CFLAGS="$CFLAGS_save"
unset CFLAGS_save
```

- 使用 `AC_PROG_CC` 检测并设置 C 编译器。
- 如果不是 GCC，则发出警告提示。
- 保存原始 CFLAGS 并恢复它们，避免被后续调用覆盖。

### 编译标志设定

```bash
EXTRA_CFLAGS="-O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
...
CFLAGS="$CFLAGS $EXTRA_CFLAGS"
```

添加优化级别 `-O3` 及必要的宏定义，例如 `_GNU_SOURCE` 和 `_FILE_OFFSET_BITS=64` 来支持大文件操作。

### 调试开关处理

```bash
#AC_ARG_ENABLE([debug], ...)
#if test "$enable_debug" = "yes"; then
#    EXTRA_CFLAGS="-g -Wall $EXTRA_CFLAGS"
#fi
```

注释掉调试模式启用逻辑，但保留了其结构作为未来扩展的基础。

### 库依赖性验证

```bash
AC_CHECK_LIB(pthread, main, [HAPPY=1], [HAPPY=0])
if test "$HAPPY" = "0"; then
    AC_MSG_WARN([Could not find the pthread library])
    AC_MSG_ERROR([Cannot continue])
fi
```

检测系统是否包含线程库（pthread），若缺失则报错终止构建流程。

### 版本变量导出

```bash
version=AC_PACKAGE_VERSION
AC_SUBST(version)
```

将软件包版本号传递给其他模块使用的变量。

## 执行输出与文件生成

```bash
AC_OUTPUT([
    Makefile
    tsunami.spec
    client/Makefile
    rtclient/Makefile
    common/Makefile
    include/Makefile
    server/Makefile
    rtserver/Makefile
    util/Makefile
])
```

该函数根据当前配置信息创建多个目标文件：
- 主 Makefile
- `.spec` 文件（用于 RPM 包管理）
- 各子目录下的 Makefile（如 client、rtclient 等）

这些文件将在后续的编译过程中使用。

## Mermaid 图表说明

以下为 configure.ac 的主要执行路径图示：

```mermaid
graph TD
    A[初始化项目] --> B[设置 Automake]
    B --> C[配置清理规则]
    C --> D[检查主机平台]
    D --> E[选择编译器]
    E --> F[设置额外编译选项]
    F --> G[检测库依赖]
    G --> H[导出版本信息]
    H --> I[生成输出文件列表]
    I --> J[结束配置过程]
```

此图表展示了 `configure.ac` 在启动时的主要步骤顺序及其相互关系。