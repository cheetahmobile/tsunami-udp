# TsunamiClient

# TsunamiClient 模块文档

## 功能概述

`TsunamiClient` 是一个命令行文件传输客户端，用于与 Tsunami 文件传输协议服务器进行通信。它支持多种操作模式，包括连接、获取单个或多个文件、列出远程共享文件等，并提供详细的性能统计和错误处理机制。

## 架构设计

### 核心组件结构

```
┌───────────────┐
│   main()        │
└──────┬────────┘
       │
┌──────▼────────────┐
│ parse_command() │
└──────┬────────┘
       │
┌──────▼────────────┐
│ dispatch      │
│ command handlers │
└──────┬────────┘
         │
    ┌────▼────┐
    │ commands│
    └────┬────┘
         │
    ┌────▼────┐
    │ protocol│
    └────┬────┘
         │
    ┌────▼────┐
    │ network │
    └────┬────┘
         │
    ┌────▼────┐
    │ io.c     │
    └─────────┘
```

### 主要模块职责

#### client/main.c
- 提供命令行界面交互逻辑
- 解析用户输入的命令字符串
- 调度分发到具体命令处理器
- 管理会话状态生命周期

#### client/command.c
- 实现具体的 CLI 命令逻辑
- 包括 `close`, `connect`, `get`, `dir`, `help`, `quit`, `set` 等核心命令
- 处理多文件获取（GET *）功能
- 启动磁盘 I/O 线程 (`disk_thread`)
- 执行数据接收循环并管理重传请求

#### client/protocol.c
- 实现 TTP 协议相关 API 接口
- 连接协商 (`ttp_negotiate`)
- 认证流程 (`ttp_authenticate`)
- 传输开始 (`ttp_open_transfer)
- UDP 端口配置 (`ttp_open_port)

#### client/network.c
- 创建 TCP 和 UDP socket 的基础网络函数
- 支持 IPv4 和 IPv6 地址解析
- 配置 socket 参数如 SO_REUSEADDR, TCP_NODELAY

#### client/io.c
- 实现将接收到的数据写入本地文件系统的核心 I/O 操作
- 使用 ring buffer 缓冲区来暂存数据块以提高效率

## 关键接口说明

### 函数签名与行为

```c
int command_close(command_t *command, ttp_session_t *session);
// 功能：关闭当前控制连接
// 返回值：0 成功，非零失败

ttp_session_t *command_connect(command_t *command, ttp_parameter_t *parameter);
// 功能：建立新的 Tsunami 控制会话
// 返回值：成功返回 session 对象指针，失败返回 NULL

int command_get(command_t *command, ttp_session_t *session);
// 功能：发起文件下载任务
// 返回值：0 成功，非零失败

int command_dir(command_t *command, ttp_session_t *session);
// 功能：列出服务器上可共享的文件列表
// 返回值：0 成功，非零失败

int command_help(command_t *command, ttp_session_t *session);
// 功能：显示帮助信息
// 返回值：0 成功

int command_quit(command_t *command, ttp_session_t *session);
// 功能：退出客户端程序
// 返回值：无返回值，直接调用 exit()

int command_set(command_t *command, ttp_parameter_t *parameter);
// 功能：设置参数或查看当前参数值
// 返回值：0 成功，非零失败
```

### 数据结构定义

```c
typedef struct {
    char **text;        // 命令文本数组
    int count;         // 文本数量
} command_t;

typedef struct {
    FILE *server;      // 控制通道流
    u_char *received;   // 已接收块位图
    u_int32_t block_count; // 总块数
    ...
} ttp_transfer_t;

typedef struct {
    ttp_transfer_t transfer;
    ttp_parameter_t *parameter;
    FILE *server;
    ...
} ttp_session_t;

typedef struct {
    const char *server_name;
    u_int16_t server_port;
    u_int16_t client_port;
    u_int32_t block_size;
    ...
} ttp_parameter_t;
```

## 执行流程详解

### 连接过程 (command_connect)

1. 从命令行获取主机名和端口号（如果提供）
2. 调用 `create_tcp_socket` 创建 TCP socket 并连接到服务器
3. 使用 `fdopen()` 将 socket 转换为 stream
4. 协商协议版本 (`ttp_negotiate`)
5. 获取用户输入的密码并进行认证 (`ttp_authenticate`)

### 下载过程 (command_get)

#### 初始化阶段：
- 设置重传请求表
- 分配接收块位图空间
- 创建 ring buffer 缓冲区用于暂存数据
- 启动 disk_thread 线程处理磁盘 I/O 操作
- 发送文件请求给服务器

#### 接收循环：
- 在 UDP socket 上监听数据包
- 解析收到的数据包头提取 block_index 和 block_type
- 如果是新块，则将其放入 ring buffer 中等待写入磁盘
- 更新已接收块位图
- 根据是否丢失块决定是否发送重传请求

#### 统计与报告：
- 计算传输速率、丢包率等性能指标
- 显示最终统计结果
- 处理目标带宽调整逻辑

### 多文件下载机制 (GET *)

当使用通配符 `*` 时，客户端会：

1. 请求所有可用文件列表
2. 遍历每个文件依次执行下载操作
3. 支持批量处理多个文件

## 内部线程模型

### disk_thread 实现说明

该线程负责将 ring buffer 中的数据块写入本地文件系统。其工作流程如下：

```c
while(1) {
    datagram = ring_peek(session->transfer.ring_buffer);
    block_index = ntohl(*((u_int32_t *) datagram));
    
    if(block_index == 0)
        return NULL;
        
    accept_block(session, block_index, datagram + 6); // 写入磁盘
    ring_pop(session->transfer.ring_buffer);       // 移除已处理的块
}
```

## 性能优化特性

### Ring Buffer 设计

- 使用环形缓冲区管理数据块缓存
- 提高了内存利用率和访问效率
- 可配置大小，默认为 4096 条目

### 重传控制策略

支持三种模式：
1. **lossless**：完全无损传输，确保所有数据都正确接收
2. **semi-lossy**：半有损传输，在指定时间窗口内允许部分数据丢失
3. **lossy**：完全有损传输，不进行任何重传

### 带宽自适应调整

在每次传输后根据实际吞吐量动态调整下一次传输的目标带宽。

## 错误处理机制

模块中广泛使用了 `warn()` 和 `error()` 函数来统一错误输出，并通过返回值明确表示操作状态。对于关键资源如 socket、file descriptor 的分配失败都会导致程序终止或返回错误码。

## 参数设置选项

| 参数名 | 类型 | 默认值 | 描述 |
|--------|------|-------|-----|
| server | string | localhost | 远程服务器地址 |
| port | int | 8000 | TCP 控制端口 |
| udpport | int | 8001 | UDP 数据端口 |
| blocksize | int | 1024 | 单个数据块大小 |
| verbose | bool | true | 是否显示详细信息 |
| transcript | bool | false | 是否记录传输日志 |
| rateadjust | bool | false | 是否自动调节目标速率 |
| error | float | 7.5% | 允许的最大错误率 |
| slowdown | fraction | 25/24 | 慢速因子 |
| speedup | fraction | 5/6 | 快速因子 |

## 编译时配置

模块支持多种编译开关以适应不同环境需求：

- VSIB_REALTIME: 支持实时 eVLBI 应用场景下的特殊 I/O 处理
- DEBUG_RETX: 启用重传调试信息输出
- RETX_REQBLOCK_SORTING: 对重传请求进行排序优化