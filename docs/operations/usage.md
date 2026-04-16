# 使用手册

## 服务端启动
```bash
./rtserver/tsunami_server \
    -p 5000 \              # 监听端口
    -w 32 \                # 窗口大小
    -t 8                   # 工作线程数
```

## 客户端连接
```bash
./rtclient/tsunami_client \
    -s 192.168.1.100 \     # 服务器地址
    -p 5000 \              # 服务器端口
    -f large_file.zip      # 要传输的文件
```