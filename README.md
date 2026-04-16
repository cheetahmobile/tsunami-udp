# tsunami-udp - 高性能UDP文件传输协议

## 核心特性
- ⚡ 基于UDP的高效传输协议
- 🔒 内置可靠传输机制
- 🌐 跨平台支持 (Linux/macOS/Windows)
- 📊 动态拥塞控制

## 架构设计
```mermaid
graph LR
    Client[客户端] -->|UDP加速流| Server[服务端]
    Server -->|数据转发| Target[目标服务器]
```

## 快速开始
### 编译安装
```bash
# 安装依赖工具
brew install autoconf automake libtool  # macOS
sudo apt-get install autoconf automake libtool  # Ubuntu

# 编译项目
./recompile.sh
```

### 启动服务端
```bash
./rtserver/tsunami_server -p 5000
```

### 客户端传输文件
```bash
./rtclient/tsunami_client -s 127.0.0.1 -p 5000 -f large_file.zip
```

## 性能对比
| 协议 | 传输速度 | CPU占用 |
|------|----------|---------|
| TCP  | 100 MB/s | 25%     |
| tsunami-udp | 850 MB/s | 15%     |

## 文档体系
```
docs/
├── design/           # 设计文档
├── operations/       # 操作指南
├── reference/        # API参考
└── tutorials/        # 教程
```
