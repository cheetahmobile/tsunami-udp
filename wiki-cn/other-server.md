# Other — server

# Other — server 模块文档

## 功能概述

`server` 模块是 Tsunami 文件传输服务器的核心配置模块。它定义了服务器运行时可调参数的默认值，并提供重置这些参数的功能。该模块主要负责初始化和管理服务器的基本配置参数，确保在启动过程中使用正确的默认设置。

## 架构设计

### 主要组件

- **常量定义**：包含所有服务器配置参数的默认值
- **reset_server 函数**：用于将服务器参数结构重置为默认值

### 参数结构体

```c
typedef struct {
    u_int32_t block_size;        /* 文件块大小 */
    u_char   *secret;            /* 共享密钥 */
    void     *client;           /* 客户端信息 */
    u_int16_t tcp_port;          /* TCP监听端口 */
    u_int32_t udp_buffer;        /* UDP发送缓冲区大小 */
    u_int16_t hb_timeout;         /* 心跳超时时间 */
    u_char verbose_yn;          /* 详细输出开关 */
    u_char transcript_yn;       /* 转录记录开关 */
    u_char ipv6_yn;             /* IPv6支持开关 */
} ttp_parameter_t;
```

### 默认值配置

| 参数名 | 默认值 | 类型 |
|---------|--------|------|
| DEFAULT_BLOCK_SIZE | 1024 | u_int32_t |
| DEFAULT_SECRET | "kitten" | u_char* |
| DEFAULT_TCP_PORT | TS_TCP_PORT | u_int16_t |
| DEFAULT_UDP_BUFFER | 20000000 | u_int32_t |
| DEFAULT_VERBOSE_YN | 1 | u_char |
| DEFAULT_TRANSCRIPT_YN | 0 | u_char |
| DEFAULT_IPV6_YN | 0 | u_char |
| DEFAULT_HEARTBEAT_TIMEOUT | 15 | u_int16_t |

## 使用方法

### 初始化服务器参数

```c
ttp_parameter_t server_params;
reset_server(&server_params);
```

这会将 `server_params` 结构体的所有字段初始化为其对应的默认值。

### 配置参数说明

- **block_size**：单个文件块的大小，默认为 1024 字节
- **secret**：客户端和服务端通信使用的共享密钥，默认为 "kitten"
- **tcp_port**：TCP 监听端口，默认使用 TS_TCP_PORT 常量
- **udp_buffer**：UDP 发送缓冲区大小，默认为 20000000 字节
- **hb_timeout**：心跳超时时间（秒），默认为 15 秒
- **verbose_yn**：是否启用详细输出，默认开启
- **transcript_yn**：是否启用转录记录，默认关闭
- **ipv6_yn**：是否启用 IPv6 支持，默认关闭

## 模块依赖关系

该模块直接依赖于：
- `tsunami-server.h` - 提供服务器相关类型定义和常量

## 版本历史

此模块包含以下重要版本变更：

- **2008/12/01**：将默认块大小从 32KB 减少到 1KB
- **2006/07/21**：添加了新的 UDP 端口定义
- **2006/07/10**：首次导入到主干中

## 注意事项

1. 所有默认值都通过宏定义存储，便于统一管理和修改
2. `reset_server()` 函数使用 `memset()` 将结构体清零后重新赋值
3. 参数结构体中的指针需要确保指向有效的内存区域
4. 此模块不包含任何内部调用或外部依赖关系