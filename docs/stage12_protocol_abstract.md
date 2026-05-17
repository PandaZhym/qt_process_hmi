# Stage12 — 协议抽象接口（IProtocol）

将 Snap7 具体适配器抽象为 IProtocol 多态接口，演示"即插即用"切换协议实现而无需求改上层代码。

## 文件结构

```
stage12_protocol_abstract/
├── CMakeLists.txt              Qt6::Widgets
├── main.cpp
│
├── i_protocol.h                抽象协议接口（纯虚类）
├── tag_mapping.h               TagMapping 数据结构（共享）
├── simulated_adapter.h/.cpp    模拟适配器实现
├── tag_manager.h/.cpp          TagManager（简化自 stage7）
├── testwindow.h/.cpp           测试主窗口
└── value_display.h/.cpp        (from process_widgets)
```

## IProtocol 接口

```cpp
class IProtocol : public QObject {
    Q_OBJECT
public:
    virtual bool    isConnected() const = 0;
    virtual QString protocolName() const = 0;

public slots:
    virtual void connectTo(const QVariantMap &params) = 0;
    virtual void disconnect() = 0;
    virtual void setMappings(const QVector<TagMapping> &mappings) = 0;
    virtual void startPolling(int intervalMs = 500) = 0;
    virtual void stopPolling() = 0;
    virtual void writeTag(const QString &tag, const QVariant &value) = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &msg);
    void tagValuesReady(const QHash<QString, QVariant> &values);
};
```

## SimulatedAdapter（模拟实现）

| 行为 | 实现 |
|------|------|
| `connectTo()` | QTimer::singleShot(1s) → m_connected=true → emit connected() |
| `disconnect()` | stopPolling() + m_connected=false → emit disconnected() |
| `startPolling()` | QTimer 周期性调用 poll() |
| `poll()` | 6 路正弦波（同 stage10/11 公式）→ emit tagValuesReady(result) |
| `writeTag()` | 写入内部 map，poll() 时覆盖正弦波值 |
| `protocolName()` | 返回 "SIM" |

## 数据管线

```
┌──────────────────────────────────────────────────────┐
│ IProtocol::tagValuesReady(QHash<QString,QVariant>)    │
│   │                                                   │
│   ▼                                                   │
│ TagManager::updateTag(name, value, valid)             │
│   │ 存储 TagValue{value, valid, timestamp}            │
│   │                                                   │
│   ▼  emit tagChanged(name, value, valid)              │
│   │                                                   │
│   ▼                                                   │
│ TestWindow::onTagChanged() → ValueDisplay::setValue() │
└──────────────────────────────────────────────────────┘
```

## 核心演示 — 多态切换

```cpp
// TestWindow 持有 IProtocol* 指针，不依赖具体类型
IProtocol *m_protocol = nullptr;

void TestWindow::onProtocolChanged(int index) {
    delete m_protocol;
    m_protocol = new SimulatedAdapter(this);  // 一行切换实现
    connect(m_protocol, &IProtocol::connected,    ...);
    connect(m_protocol, &IProtocol::tagValuesReady, ...);
}
```

上层 TagManager 和 ValueDisplay 完全不知协议变更，仅依赖 IProtocol 信号。

## UI 布局

- 顶部：协议选择下拉框 + 连接/断开按钮 + LED 状态指示
- 中部：6 个 ValueDisplay 实时显示标签值
- 底部：QTextEdit 事件日志（连接/断开/错误/轮询 时间戳）

---

*Status: Compiled & Running | Qt6::Widgets | No PLC*
