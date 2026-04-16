# Other — util

# Other — util 模块文档

## 概述

`Other — util` 模块是一组用于支持 Tsunami 软件包的工具程序和脚本。这些工具主要用于性能测试、数据处理以及实验记录自动化等任务，为系统提供底层支持功能。

该模块包含多个独立可执行文件或脚本，如读写吞吐量基准测试器（readtest, writetest）、合并扫描时间序列的脚本生成器（makeRecexpt.sh）及其针对 Tsunami 的增强版本（makeRecexpt-tsunami.sh），还有用于分析传输速率的绘图脚本（tsuc2plot.sh）等。

## 核心组件与功能说明

### 性能测试工具

#### `readtest.c`
- 功能：对磁盘读取速度进行基准测试。
- 使用方式：
```bash
./readtest filename [block_size]
```
- 参数：
    - `filename`: 待测文件路径。
    - `block_size`: 块大小，默认为 32678 字节。
- 输出信息包括开始时间、结束时间、间隔时间和读取速度 Mbps。

#### `writetest.c`
- 功能：对磁盘写入速度进行基准测试。
- 使用方式：
```bash
./writetest filename [block_size]
```
- 参数：
    - `filename`: 创建并写入的目标文件名。
    - `block_size`: 写入块大小，默认为 32678 字节。
- 测试会持续到写满约 5GB 数据，并输出写入耗时及速度。

#### `fusereadtest.c`
- 功能：模拟 DiFX 接收环中的随机延迟读取行为，用于评估在不同延迟下的实际吞吐表现。
- 特点：使用随机睡眠来模仿网络波动情况。
- 使用方法：
```bash
./fusereadtest filename [readbytes]
```

### 数据转换与解析工具

#### `bskp.c`
- 功能：实现“字节跳过”逻辑，从输入流中每四个字节取出前三个字节输出。
- 应用场景：适用于特定格式的数据预处理流程。

#### `dump.c`
- 功能：以三出四的方式打印输入数据的十六进制内容。
- 输出格式示例：
```
0000: FF EE DD CC ...
```

### 自动化脚本生成器

#### `makeRecexpt.sh`
- 功能：根据 `.snp` 文件自动生成实验记录脚本（`.sh` 脚本）。
- 输入参数：
    - `snapfile.snp`: DRUDG 工具生成的调度文件。
    - `experimentName`: 实验名称。
    - `stationID`: 站点标识符。
- 输出结果是一个名为 `recexpt_${EXPT}_${STATION}.sh` 的 shell 脚本。

#### `makeRecexpt-tsunami.sh`
- 功能：基于 `.snp` 文件创建 Tsunami 客户端侧的自动记录脚本。
- 需要手动编辑的部分：
    - 修改 `recpass-tsunami` 中服务器地址和密码设置。
- 输出脚本命名为 `recexptsunami_${EXPT}_${STATION}.sh`。

### 记录控制脚本

#### `recexpt-tsunami.head`, `recexpt-tsunami.tail`
- 功能：作为 `makeRecexpt-tsunami.sh` 自动生成脚本的一部分，定义了头部和尾部结构。
- 包含变量如 `HEADROOM` 和 `SKIPOLDSCANS` 控制启动安全余量和是否忽略已过期扫描。

#### `recpass-tsunami`
- 功能：远程获取单个扫描数据并将其保存至本地目录。
- 参数：
    - `scanName`: 扫描名。
    - `recordTimeInSecs`: 录制时间长度（秒）。
- 依赖项：
    - 远程 Tsunami 服务端配置。
    - 指定根目录 (`ROOTDIR`) 存储路径。
    - 设置录制速率 (`RATE`)。

### 可视化分析工具

#### `tsuc2plot.sh`
- 功能：将 Tsunami 客户端传输日志（`.tsuc`）提取为可用于 Octave/GNUPlot 绘图的数据格式。
- 使用方法：
```bash
./tsuc2plot.sh tsunamiClientTranscript.tsuc
```
- 输出图形显示接收率、丢包率与累计速率随时间的变化趋势。

#### `tsucmatlabplot.m`
- 功能：MATLAB 函数用于绘制由 `tsuc2plot.sh` 提取的日志数据。
- 接口参数：
    - `file`: 日志文件路径。
    - `plottitle`: 图形标题字符串。

## 架构关系图

```mermaid
graph TD
    A[readtest] --> B{Disk Read Benchmark}
    C[writetest] --> D{Disk Write Benchmark}
    E[fusereadtest] --> F{DiFX Ring Simulation}
    G[bskp] --> H[Byte Skip Tool}
    I[dump] --> J{Hex Dump Utility}
    K[makeRecexpt.sh] --> L{Auto Gen Script}
    M[makeRecexpt-tsunami.sh] --> N{Auto Gen Tsunami Script}
    O[recpass-tsunami] --> P{Tsunami Remote Recorder}
    Q[tsuc2plot.sh] --> R{Log Data Extractor}
    S[tsucmatlabplot.m] --> T{Rate Plotter}

    subgraph Performance Tools
        A
        C
        E
    end

    subgraph Data Processing
        G
        I
    end

    subgraph Automation Scripts
        K
        M
    end

    subgraph Visualization
        Q
        S
    end
```

## 跨模块交互说明

本模块中的程序主要通过以下方式与其他部分交互：

1. **系统调用**：
   - `gettimeofday()` 和 `filetime_to_unix_epoch()` 等函数被多个测试程序使用，以获取当前时间戳并转换成 Unix 时间戳。
   - 所有性能测试工具都依赖于 common 库中提供的通用时间处理接口。

2. **外部脚本执行**：
   - `makeRecexpt-tsunami.sh` 会生成新的 shell 脚本，并在其中调用 `date`, `sed`, `awk` 等命令行工具来解析 `.snp` 文件内容。
   - `tsuc2plot.sh` 利用了 `octave` 来进行绘图分析。

3. **配置文件读写**：
   - 大多数脚本（如 `recpass-tsunami`）需要用户手动修改某些变量值，例如服务器地址、RAID 根目录等。
   - 自动化脚本生成器根据输入的调度信息动态构建控制逻辑。

4. **I/O 操作与设备驱动**：
   - 如 `fopen`, `fwrite`, `fread` 等标准库函数用于访问本地磁盘或 `/dev/vsib` 设备。
   - `wr` 工具是实际负责将数据写入 RAID 存储系统的底层组件之一。

5. **网络通信**：
   - `iperf-tester.sh` 使用 iperf 进行 UDP 测试，模拟真实环境下的网络传输情况。
   - `recpass-tsunami` 中涉及远程连接和数据下载操作，需配合 Tsunami 客户端应用完成。

## 编译与运行指南

### 编译所有工具
```bash
cd util/
make clean && make
```
> 注意：如果未启用大文件支持（LFS），请确保编译时包含 `-D_FILE_OFFSET_BITS=64` 或类似选项。

### 示例运行命令

#### 性能测试
```bash
./readtest /path/to/testfile.dat
./writetest testfile.dat
```

#### 数据处理
```bash
cat input.data | ./bskp > output.data
./dump < input.data
```

#### 实验记录自动化
```bash
./makeRecexpt-tsunami.sh demo.snp EURO85 on
chmod +x recexptsunami_EURO85_on.sh
./recexptsunami_EURO85_on.sh
```

#### 日志可视化
```bash
./tsuc2plot.sh tsunamiClientTranscript.tsuc
```

## 开发建议

- 对于新添加的功能，请保持代码风格一致性和注释规范。
- 所有性能测试程序应尽量避免硬编码路径，提高可移植性。
- 建议对 `makeRecexpt*.sh` 脚本增加错误检查机制以增强健壮性。
- 可考虑使用更现代的 shell 语法替代老旧的 bash 表达式，提升维护效率。