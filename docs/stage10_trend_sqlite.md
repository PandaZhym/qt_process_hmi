# Stage10 — 趋势曲线 + SQLite 持久化

趋势曲线控件 + SQLite 数据持久化，合并为一个独立项目。

## 文件结构

```
stage10_trend_sqlite/
├── CMakeLists.txt              Qt6::Widgets + Qt6::Sql
├── main.cpp
├── testwindow.h/.cpp           1100×700，6 标签正弦波 + 100ms QTimer
│
├── data_logger.h/.cpp          SQLite 工作线程 + 缓冲批量写入
├── trend_chart.h/.cpp          趋势曲线控件（自绘）
└── build/                      数据库文件 trend_data.db 运行时生成
```

## 核心类

### DataLogger（SQLite 持久化）

**架构**: QObject → `moveToThread()` 到子线程，数据库操作彻底隔离

**线程模型**:
```
Main Thread                          Worker Thread (logger_conn)
     │                                     │
     ├─ logValue(tag, val, ts) ──(queued)──→ m_buffer.append()
     │                                     │
     │                              QTimer 500ms → flushNow()
     │                                     │
     │                              Batch INSERT + Transaction
     │                                     │
     ├─ queryHistory(tag, from, to) ──→    │
     │                              SELECT + emit queryResult()
     ◄── queryResult(tag, data) ──(signal)─┘
```

**表结构**:
```sql
CREATE TABLE IF NOT EXISTS tag_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tag_name TEXT NOT NULL,
    value REAL NOT NULL,
    timestamp INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_tag_time ON tag_history(tag_name, timestamp);
```

**写入策略**: QMutex 保护缓冲区，满 1000 条或每 500ms 批量 flush，一个 SQLite transaction 包所有 INSERT。

### TrendChart（趋势曲线控件）

**显示特性**:
- `addCurve(name, color)` 注册多曲线
- X 轴 = 时间，Y 轴 = 数值
- 网格背景 + 坐标轴刻度标注
- Y 轴自动缩放或手动设置范围
- 图例（右上角，可点击显隐）

**交互**:
| 操作 | 效果 |
|------|------|
| 鼠标滚轮 | 缩放时间范围（1min ~ 60min，×1.3 系数） |
| 左键拖拽 | 平移时间轴 |
| 双击 | 重置到默认 10min + 自动 Y 轴 |

**双模式**:
| 模式 | 数据源 | 缓冲 |
|------|--------|------|
| 实时 | `appendData(tag, ts, val)` | 环形缓冲 6000 点（60 min @ 100ms） |
| 历史 | `loadHistory(tag, data)` + `setHistoryRange(from, to)` | 静态显示 |

**绘制逻辑**:
- `dataToPixel()` / `pixelToTime()` — 屏幕坐标映射
- `niceTimeStep()` / `niceValueStep()` — 刻度线取整到优美值
- QPainterPath 连点多曲线绘制

## TestWindow 数据流

```
100ms QTimer → 6 路正弦波
    ├→ TrendChart::appendData()  (实时曲线)
    └→ DataLogger::logValue()    (SQLite 持久化)

"查询历史" → DataLogger::queryHistory()
    → queryResult信号 → TrendChart::loadHistory()
    → TrendChart::setHistoryRange()
    → TrendChart::setRealtimeMode(false)

"切回实时" → TrendChart::setRealtimeMode(true)
```

---

*Status: Compiled & Running | Qt6::Widgets + Qt6::Sql | No PLC*
