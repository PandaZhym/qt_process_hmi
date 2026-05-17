# Stage13 — JSON 配置驱动画面

架构方案 Route B：画面由 JSON 配置文件定义，运行时解析生成控件树，替代硬编码 C++ 布局。

## 文件结构

```
stage13_json_config/
├── CMakeLists.txt              Qt6::Widgets + AUTORCC
├── main.cpp
│
├── config_parser.h/.cpp        JSON → ScreenConfig{WidgetDef[]}
├── widget_factory.h/.cpp       类型注册表 + create() + applyProperties()
├── data_binding.h/.cpp         Setter 注册表 + ExprEval + 信号连线
├── screen_builder.h/.cpp       编排上述组件 → buildFromJson() → QWidget*
├── testwindow.h/.cpp           加载 2 个 JSON，NavBar 切换画面
│
├── sim_data_manager.h/.cpp     (from stage11)
├── nav_bar.h/.cpp              (from stage11)
├── (process_widgets)           tank / pump / valve / pipe / value_display
│
├── configs/
│   ├── screen_process.json     流程画面（Tank+Pump+Valve+Pipe+ValueDisplay）
│   └── screen_monitor.json     监控仪表盘（6×ValueDisplay 卡片）
└── resources/
    └── stage13.qrc              Qt 资源（嵌入 JSON 到二进制）
```

## JSON 配置格式

```json
{
    "name": "Process Flow",
    "version": 1,
    "widgets": [
        {
            "id": "tank_main",
            "type": "TankWidget",
            "x": 30, "y": 30, "width": 160, "height": 240,
            "properties": {
                "label": "TK-101", "alarmLow": 10, "alarmHigh": 90
            },
            "bindings": [
                {"tag": "TANK", "property": "level", "transform": "val * 2.5"}
            ]
        }
    ]
}
```

**type 枚举**: `TankWidget` | `PumpWidget` | `ValveWidget` | `PipeWidget_H` | `PipeWidget_V` | `ValueDisplay`

**properties**: 静态属性（构建时设置一次） — label, unit, decimals, alarmLow, alarmHigh, setpoint

**bindings**: 动态绑定 — tag → property，可选 `transform` 表达式

## 四个核心类

### 1. ConfigParser
```
JSON file → QFile → QJsonDocument → QVector<WidgetDef>
```
每个 WidgetDef 包含：id, type, x/y/width/height, properties(QJsonObject), bindings[]

### 2. WidgetFactory
```
Type Registry:  QHash<QString, CreatorFunc>
                "TankWidget" → new TankWidget(p)
                "PipeWidget_H" → new PipeWidget(Horizontal, p)
                ...
create()        → QWidget*
applyProperties() → per-type dispatch: setLabel/setUnit/setAlarmLimits/setSetpoint
```

### 3. DataBinding
```
Setter Registry:  QHash<"Type.prop", SetterFunc>
                  "TankWidget.level" → setLevel(qBound(0, v, 100))
                  "PumpWidget.running" → setRunning(v > 0.5)
                  "ValueDisplay.value" → setValue(v)
                  ... (10 entries)

bindWidget():     for each binding:
                    connect(SimDataManager::valueChanged, widget,
                      lambda: filter tag → ExprEval::evaluate() → setter())

applyInitialValues(): read current value → evaluate → setter()
```

### 4. ExprEval（递归下降表达式求值器）

| 优先级 | 操作符 |
|--------|--------|
| 1 | `\|\|` |
| 2 | `&&` |
| 3 | `<` `>` `<=` `>=` `==` `!=` |
| 4 | `+` `-` |
| 5 | `*` `/` |
| 6 | Unary `-` `!` |
| 7 | `val` `NUM` `(expr)` |

Boolean 返回 1.0 / 0.0，与算术无缝混合。除零返回 0 + qWarning。

### ScreenBuilder 编排流程

```
buildFromJson(path):
  1. ConfigParser::parse(path) → ScreenConfig
  2. 创建容器 QWidget (title + QFrame canvas)
  3. for each WidgetDef:
       a. widget = WidgetFactory::create(type, canvas)
       b. widget->setGeometry(x, y, w, h)
       c. WidgetFactory::applyProperties(type, widget, properties)
       d. DataBinding::applyInitialValues(sim, widget, type, bindings)
       e. DataBinding::bindWidget(sim, widget, type, bindings)
  4. return container
```

## 两个演示 JSON 配置

### screen_process.json
10 个控件：TankWidget(1) + PipeWidget_H(2) + PumpWidget(1) + ValveWidget(1) + ValueDisplay(5)
- Tank 液位绑定：`TANK * 2.5`
- Tank 报警绑定：`val < 10 || val > 90`
- Pipe 流动绑定：`PUMP > 5`

### screen_monitor.json
6 个 ValueDisplay 组成 3×2 网格仪表盘
- TANK 变换：`val / 40 * 100`

---

*Status: Compiled & Running | Qt6::Widgets | JSON in Qt Resources | No PLC*
