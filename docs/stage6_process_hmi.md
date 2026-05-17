# Stage6 — 流程 HMI 集成（控件 + PLC + 深色主题）

首个将 process_widgets 控件组与真实 PLC 数据打通的独立运行项目。

## 文件结构

```
stage6_process_hmi/
├── CMakeLists.txt              Qt6::Widgets + Snap7
├── main.cpp
├── mainwindow.h/.cpp           深色主题 + 流程画面布局 + 线程管理
├── plc_worker.h/.cpp           定时轮询 + 固定 DB 地址读取 + 数据解析
├── snap7.h/.cpp                Snap7 C++ wrapper
└── ../process_widgets/         复用 5 个基础控件
    ├── tank_widget.h/.cpp
    ├── pump_widget.h/.cpp
    ├── valve_widget.h/.cpp
    ├── pipe_widget.h/.cpp
    └── value_display.h/.cpp
```

## 核心改进（对比 stage5）

| 方面 | stage5 | stage6 |
|------|--------|--------|
| 数据展示 | 表格行文本 | 图形化控件（罐、泵、阀、管道、仪表） |
| 监控方式 | 手动配置监控项 | 固定 DB 地址，连接即开始轮询 |
| UI 风格 | 系统默认 | 自主设计的深色工业主题（QSS） |
| 数据传递 | 逐个信号发 UI，UI 解析 | worker 直接解析为业务值，一次信号更新全部控件 |

## 核心类

### PlcWorker（PLC 通讯 + 轮询）

**固定 DB 地址映射**（以 DB1 为例）：

| 变量 | DB 偏移 | 类型 | 对应控件 |
|------|---------|------|---------|
| 罐体液位 | DBD0 | FLOAT | TankWidget |
| 泵转速 | DBD4 | FLOAT | PumpWidget |
| 阀开度 | DBD8 | FLOAT | ValveWidget |
| 温度 | DBD12 | FLOAT | ValueDisplay |
| 压力 | DBD16 | FLOAT | ValueDisplay |
| 流量 | DBD20 | FLOAT | ValueDisplay |
| 泵运行 | DBX24.0 | BOOL | PumpWidget |

**数据流**：
```
QTimer 500ms → pollAllItems()
  → 逐项 DBRead(DB1, offset, 4)
  → readFloatFromDB / readBoolFromDB 解析
  → emit processDataUpdated(tankLevel, pumpSpeed, pumpRun,
                             valveOpen, temp, pressure, flow)
  → MainWindow::onProcessDataUpdated()
    → m_tank->setLevel() / m_pump->setSpeed() / ...
```

### MainWindow（画面 + 数据刷新）

**布局**（1050×680）：
```
┌──────────────────────────────────────┐
│ IP [    ] Rack Slot DB [连接][断开]  ◉ 状态  CPU: RUN        │ ← 连接控制行
├──────────────────────────────────────┤
│  ┌──────┐  ┌──────┐  ┌──────┐       │ ← 仪表显示栏
│  │ 温度  │  │ 压力  │  │ 流量  │       │   (ValueDisplay ×3)
│  └──────┘  └──────┘  └──────┘       │
├──────────────────────────────────────┤
│  ┌─────┐  ═══  ┌──────┐  ═══  ┌────┐ ═══ │ ← 流程图画布
│  │ TANK │══╡■■■│ PUMP │■■■╞══│VALVE│══╡  │   (绝对定位)
│  └─────┘  ═══  └──────┘  ═══  └────┘ ═══ │
└──────────────────────────────────────┘
```

**深色主题**：全部通过 QSS 样式表实现，无外部 qss 文件依赖。
背景色 `#1a1d23`，控件色 `#2a2d33`，强调色 `#2d5a8c`（蓝）。

**管道逻辑**：
```cpp
// 管道流动状态 = 泵在运行 且 有相应条件
m_pipe1->setFlowing(pumpRun && tankLevel > 0);          // 罐→泵
m_pipe2->setFlowing(pumpRun && valveOpen > 5);           // 泵→阀
m_pipe3->setFlowing(pumpRun && valveOpen > 5);           // 阀→出口
```

---

*Status: Compiled & Running | Qt6::Widgets | Real PLC (S7-1200) | Dark Theme*