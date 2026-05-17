# Qt6 Process HMI Framework — Architecture & Project Overview

## 项目概述

基于 Qt6 + C++17 的工业过程监控 HMI 框架，涵盖从 PLC 通讯到 UI 渲染的完整三层架构。所有项目采用 **纯 QPainter 自绘** 渲染方案，暗色主题，独立可编译。

- **Build system**: CMake + Ninja + MinGW (GCC 13.1)
- **Qt version**: 6.11.0
- **C++ standard**: C++17
- **Platform**: Windows 11

---

## 三层架构

```
┌─────────────────────────────────────────────────┐
│  Layer 3: Widgets （画面层）                      │
│  TankWidget · PumpWidget · ValveWidget           │
│  PipeWidget · ValueDisplay · TrendChart           │
│  AlarmBanner · AlarmListWidget · NavBar           │
│  OverviewCard · HmiScreen                        │
├─────────────────────────────────────────────────┤
│  Layer 2: Data Model （数据层）                    │
│  TagManager · SimDataManager · AlarmManager       │
│  DataLogger · DataBinding                        │
├─────────────────────────────────────────────────┤
│  Layer 1: Communication （通讯层）                 │
│  IProtocol · SimulatedAdapter · Snap7Adapter      │
│  ConfigParser · JSON config files                 │
└─────────────────────────────────────────────────┘
```

**关键设计决策**（来自架构方案讨论记录）：

| 决策 | Route A（当前） | Route B（已实现） |
|------|----------------|-------------------|
| 画面构成 | C++ 硬编码布局 | **stage13**: JSON 配置文件驱动 |
| 通讯协议 | Siemens Snap7 | **stage12**: IProtocol 多态接口 |
| 数据存储 | 内存 | **stage10**: SQLite 持久化 |

---

## 项目地图

```
qt_process_hmi/
├── process_widgets/          共享控件库（5 种工业控件）
├── docs/                     项目文档（.md + .pdf）
├── 架构方案讨论记录.md         架构设计原始讨论
│
├── stage6_process_hmi/       PLC 直连原型（Snap7，单线程）
├── stage7_mvp_framework/     MVP 架构（Snap7Adapter + TagManager + Widgets）
│
├── stage8_ui_enhance/        纯 QPainter 渲染增强（独立 UI 测试）
├── stage8_svg_enhance/       SVG + QPainter 混合渲染（独立 UI 测试）
├── stage9_alarm_mgmt/        报警管理系统（独立测试）
├── stage10_trend_sqlite/     趋势曲线 + SQLite 持久化（独立测试）
├── stage11_multi_screen/     多屏导航框架（独立测试）
├── stage12_protocol_abstract/ 协议抽象接口（独立测试）
└── stage13_json_config/      JSON 配置驱动画面（独立测试）
```

---

## 阶段总览

### Stage 6-7: PLC 通讯原型（Siemens Snap7）
- stage6: 硬编码 7 个标签，直接 DBRead
- stage7: TagMapping 抽象 + TagManager 数据中枢 + 双线程架构

### Stage 8: UI 美化（两套方案对比）
- **8_ui_enhance**: 纯 QPainter 增强渲染（多段渐变 / 金属框 / 发光 / 玻璃反射 / LED / 动画）
- **8_svg_enhance**: QSvgRenderer + QPainter 混合渲染（SVG 静态底图 + QPainter 动态叠加）

### Stage 9: 报警管理
- 4 级报警（HH/H/L/LL），2% 回差防抖，4 种确认状态
- AlarmManager + AlarmBanner + AlarmListWidget
- 已恢复未确认报警需操作员确认

### Stage 10: 趋势曲线 + SQLite
- TrendChart: 多曲线自绘、滚轮缩放(1~60min)、拖拽平移、实时/历史双模式
- DataLogger: 子线程 SQLite 批量写入，queryHistory 查询

### Stage 11: 多屏导航
- NavBar: QPainter 自绘导航栏（按钮高亮、时钟、报警红点）
- HmiScreen: 画面基类（onEnter/onLeave/onTick 生命周期）
- 5 个演示画面：总览 / 流程 / 报警 / 趋势 / 设置

### Stage 12: 协议抽象
- IProtocol: 纯虚接口（connectTo/disconnect/setMappings/startPolling/writeTag + 4 signals）
- SimulatedAdapter: 模拟 1 秒连接延迟 + 正弦波数据
- IProtocol → TagManager → ValueDisplay 完整管线

### Stage 13: JSON 配置驱动
- ConfigParser: JSON → WidgetDef 解析
- WidgetFactory: 类型注册表工厂（6 种控件）
- DataBinding: Setter 注册表 + ExprEval 表达式求值器
- Data flow: JSON → ConfigParser → WidgetFactory → DataBinding → SimDataManager
- 2 个演示 JSON 配置文件嵌入 Qt 资源

---

## 共享控件库 `process_widgets/`

| 控件 | 功能 | 可变属性 |
|------|------|---------|
| TankWidget | 罐体液位 + 背景梯度 + 液位动画 | level(0-100), alarm(bool), setpoint, label |
| PumpWidget | 离心泵 + 旋转叶轮动画 | speed(0-100), running(bool), label |
| ValveWidget | 阀门本体 + 开度三角指示 | opening(0-100), label |
| PipeWidget | 管道 + 流动粒子动画 | flowing(bool), direction(H/V) |
| ValueDisplay | 数值面板 + 报警边框 | value, label, unit, decimals |

所有控件均 `QWidget` + `paintEvent()` 自绘，部分带 `Q_PROPERTY` + `QPropertyAnimation` 平滑过渡。

---

## 数据流模式

```
                      ┌─────────────┐
                      │  QTimer     │  (100ms / 200ms)
                      └──────┬──────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              │              ▼
      SimulatedAdapter  SimDataManager  IProtocol::poll()
      (stage12)         (stage11/13)    (stage12)
              │              │              │
              └──────┬───────┘              │
                     │                      │
              tagValuesReady        valueChanged
                     │                      │
                     ▼                      ▼
               TagManager            DataBinding
                     │                 (stage13)
               tagChanged                 │
                     │                    │
                     ▼                    ▼
               Widgets ◄──────────────────┘
              (setValue / setLevel / setSpeed / ...)
```

---

## 验证构建

所有项目使用统一构建命令：

```bash
cd stage{N}_{name}
cmake -B build -G "Ninja" \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
cmake --build build
cd build
C:/Qt/6.11.0/mingw_64/bin/windeployqt.exe Stage{N}_{Name}.exe
./Stage{N}_{Name}.exe
```

---

*Generated 2026-05-17 | Qt6 + C++17 + QPainter | Process HMI Learning Project*
