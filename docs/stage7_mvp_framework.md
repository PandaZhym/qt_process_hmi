# Stage7 — MVP 三层架构（TagManager + Snap7Adapter）

在 stage6 基础上抽取出**标准化三层架构**，将通讯层与画面层彻底解耦。

## 文件结构

```
stage7_mvp_framework/
├── CMakeLists.txt              Qt6::Widgets + Snap7
├── main.cpp
├── mainwindow.h/.cpp           画面组装 + 标签绑定 + 数据路由
├── tag_manager.h/.cpp          标签管理器（Layer 2，主线程）
├── snap7_adapter.h/.cpp        通讯适配器（Layer 1，子线程）
├── snap7.h/.cpp                Snap7 C++ wrapper
└── ../process_widgets/         复用 5 个基础控件
```

## 三层架构

```
┌───────────────────────────────────────┐
│  Layer 3: 画面层 (MainWindow)          │
│  ┌──────┐ ┌──────┐ ┌──────┐          │
│  │ Tank │ │ Pump │ │Valve │ ...       │  ← process_widgets
│  └──────┘ └──────┘ └──────┘          │
│         ↑ tagChanged() 信号            │
├─────────┼─────────────────────────────┤
│  Layer 2: 数据层 (TagManager)          │  主线程
│  tagChanged("TANK_LEVEL", 85.3)       │
│         ↑ updateTag()                  │
├─────────┼─────────────────────────────┤
│  Layer 1: 通讯层 (Snap7Adapter)        │  子线程
│  ReadMultiVars() → parseValue()       │
│         ↑ QTimer 轮询                  │
├───────────────────────────────────────┤
│  S7-1200 PLC (端口 102)               │
└───────────────────────────────────────┘
```

**对比 stage6**：stage6 的 PlcWorker 既管通讯又管地址映射，所有逻辑混在一起。stage7 拆成两层——Snap7Adapter 只管"怎么读"，TagManager 只管"标签→控件"的路由。

## 核心类

### Snap7Adapter（通讯适配器）

**关键改进**：使用 Snap7 的 `ReadMultiVars` 一次读取所有标签，而非逐条 `DBRead`。

```cpp
// 传统方式（stage6）：逐条读，每条一次网络往返
for each tag: DBRead(db, offset, size)

// ReadMultiVars（stage7）：一次请求读全部，显著减少延迟
QVector<TS7DataItem> items;
// ...填充所有标签的 Area/Start/Amount/pdata...
m_client->ReadMultiVars(items.data(), n);
// 一次 TCP 往返取回所有数据
```

**标签映射配置**（`TagMapping`）：
```cpp
struct TagMapping {
    QString tagName;    // "TANK_LEVEL"
    int area;           // S7AreaDB
    int dbNum;          // 6
    int start;          // 0 (字节偏移)
    int size;           // 4 (字节数)
    int valType;        // 6 = FLOAT32
};
```

**信号**：`tagValuesReady(QHash<QString, QVariant> values)` — 一次把本轮所有标签值打包发回 UI 线程。

### TagManager（标签管理器）

**职责**：充当标签→控件的中间注册表，控件不知道 PLC 地址，只知道标签名。

```cpp
// 通讯层写入数据
m_tagMan->updateTag("TANK_LEVEL", 85.3);
    → 存入 m_tags["TANK_LEVEL"]
    → emit tagChanged("TANK_LEVEL", 85.3, true)

// 画面层响应
onTagChanged("TANK_LEVEL", 85.3, true)
    → m_tank->setLevel(85.3)
```

**为什么用 TagManager**：
- 控件不需要知道数据来自 PLC 还是模拟器
- 后续换通讯协议（Modbus、OPC UA）只需换 Adapter，TagManager 和画面不动
- 支持时间戳和 valid 标记，可用于断线处理

### MainWindow（画面 + 数据路由）

**`setupArchitecture()`** — 组装三层：
```cpp
m_tagMan  = new TagManager(this);      // Layer 2，主线程
m_thread  = new QThread(this);
m_adapter = new Snap7Adapter();         // Layer 1，子线程
m_adapter->moveToThread(m_thread);

// 跨线程：Adapter(子) → MainWindow(主) → TagManager(主)
connect(m_adapter, &Snap7Adapter::tagValuesReady,
        this, &MainWindow::onTagValuesReady);
```

**`onTagValuesReady()`** — 数据路由核心：
```cpp
void MainWindow::onTagValuesReady(const QHash<QString, QVariant> &values)
{
    // ① 写入 TagManager（触发 tagChanged → 控件刷新）
    for (auto it = values.begin(); it != values.end(); ++it)
        m_tagMan->updateTag(it.key(), it.value());
}
```

**管道逻辑**：已改用 TagManager 读取关联标签值判断流动状态，而非依赖传入的参数。

---

*Status: Compiled & Running | Qt6::Widgets | Real PLC (S7-1200) | Dark Theme*
*Key: ReadMultiVars 批量读取 + TagManager 标签抽象 + 三层解耦*