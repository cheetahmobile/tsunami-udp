# 新手入门指南

## 环境搭建
```mermaid
graph LR
    A[开发机] --> B[安装工具链]
    B --> C[获取代码]
    C --> D[编译项目]
```

## 第一个示例
```bash
# 启动服务端
./rtserver/tsunami_server -p 5000 &

# 传输文件
./rtclient/tsunami_client -s 127.0.0.1 -p 5000 -f test.jpg
```
![文件传输示意图](images/file-transfer.png)

## 常见问题
| 问题 | 解决方案 |
|------|---------|
| 编译失败 | 检查 autotools 版本 |
| 连接超时 | 检查防火墙设置 |