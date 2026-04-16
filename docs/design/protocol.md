# 协议设计规范

## 数据包格式
```c
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;        // 魔数 0x5453554D (TSUN)
    uint32_t seq;          // 序列号
    uint32_t ack;          // 确认号
    uint16_t window;       // 窗口大小
    uint8_t flags;         // 控制标志
    uint8_t reserved;      // 保留字段
    uint16_t checksum;     // CRC16校验和
} tsunami_packet_header_t;
#pragma pack(pop)
```

## 控制标志
| 位 | 名称 | 描述 |
|----|------|------|
| 0 | SYN | 连接请求 |
| 1 | ACK | 确认响应 |
| 2 | FIN | 连接终止 |
| 3 | RST | 连接重置 |