# Stage9 — 报警管理系统

完整的工业报警管理系统，包含 4 级报警、回差防抖、确认生命周期、闪烁指示。

## 文件结构

```
stage9_alarm_mgmt/
├── CMakeLists.txt              Qt6::Widgets
├── main.cpp
├── testwindow.h/.cpp           950×620，6 标签正弦波 + 100ms QTimer
│
├── alarm_manager.h/.cpp        报警逻辑引擎
├── alarm_banner.h/.cpp         顶部报警横幅（脉动红底，36px）
├── alarm_list_widget.h/.cpp    自绘报警列表（28px 行高）
└── tag_display.h/.cpp          标签值显示
```

## 核心类

### AlarmManager（报警逻辑引擎）

**数据结构**:
```cpp
enum class AlarmType { HH, H, L, LL };

struct AlarmCondition {
    QString tagName;
    double hhLimit, hLimit, lLimit, llLimit;  // NaN = 不启用
    bool enabled = true;
};

struct AlarmRecord {
    int id;
    QString tagName;     AlarmType type;
    double limit;        double actualValue;
    QDateTime timestamp;
    bool acknowledged;   bool active;
};
```

**核心机制**:
- 4 条独立阈值检测线（HH/H/L/LL），NaN 表示未启用
- 2% 回差（Hysteresis）：高报触发 > limit，清除 < limit×0.98；低报触发 < limit，清除 > limit×1.02
- 5 标签 + 阈值定义：TEMP(HH>90,H>80,L<10)，PRESS(HH>3.5,H>2.8)，TANK(L<15,LL<5)，PUMP(HH>95,H>90)，FLOW(L<5)
- `acknowledgeAll()` / `acknowledgeId()` 确认操作

**信号**:
- `alarmTriggered(AlarmRecord)` / `alarmCleared(id)` / `alarmAcknowledged(id)`
- `countsChanged(activeCount, unackedCount)`

### 报警确认生命周期（4 种状态）

| 状态 | 活跃 | 已确认 | 视觉效果 |
|------|------|--------|---------|
| ● 未确认 | ✓ | ✗ | 红色背景闪烁 |
| 已确认 | ✓ | ✓ | 橙色文字 |
| ▲ 待确认 | ✗ | ✗ | 橙色背景闪烁 |
| √ 已恢复 | ✗ | ✓ | 绿色文字 |

**关键设计**：报警自动恢复后不清除未确认计数，操作员仍需确认。

### AlarmBanner（顶部横幅）
- 固定 36px 高，无未确认报警时隐藏
- 脉动红色背景（400ms QTimer，sin 强度）
- 显示：未确认总数 + 已恢复待确认数 + 最新报警详情
- 点击 → 确认全部

### AlarmListWidget（报警列表）
- 自绘列表，28px 行高
- 色标 + 标签名 + 级别(HH/H/L/LL) + 限值 + 当前值 + hh:mm:ss + 状态
- 鼠标悬停高亮，点击单条确认
- 500ms 闪烁定时器

## 遇到的错误

1. `m_alarmMan` = nullptr → SEGFAULT：构造函数中忘记 `new AlarmManager(this)`。
2. 自动恢复 → `m_unackedCount--` 导致横幅消失：移除 clear 分支中的减计数逻辑。

---

*Status: Compiled & Running | Qt6::Widgets | No PLC*
