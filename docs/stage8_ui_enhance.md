# Stage8 — UI 美化测试（纯 QPainter 方案）

纯 QPainter 增强渲染方案。新建独立 Qt 项目，无 PLC 通讯。

## 文件结构

```
stage8_ui_enhance/
├── CMakeLists.txt              Qt6::Widgets
├── main.cpp
├── testwindow.h/.cpp           1050×680，80ms QTimer 正弦波模拟
│
├── widget_render_helpers.h/.cpp  9 个共享渲染函数
├── tank_widget.h/.cpp            11 层 paintEvent 罐子控件
├── pump_widget.h/.cpp            离心泵控件（呼吸发光 + 速度弧）
├── valve_widget.h/.cpp           阀门控件（QPropertyAnimation 平滑开度）
├── pipe_widget.h/.cpp            管道控件（3D 梯度 + 粒子流动）
└── value_display.h/.cpp          数值显示控件（LCD 面板 + 闪烁）
```

## 共享渲染函数 `widget_render_helpers.h`

| 函数 | 功能 |
|------|------|
| `drawMetallicFrame()` | 多段渐变金属边框 |
| `drawGlowEffect()` | 径向发光效果 |
| `drawGlassReflection()` | 玻璃高光反射 |
| `drawLEDIndicator()` | LED 状态灯（内圈+外圈发光） |
| `drawRulerScale()` | 刻度尺标注 |
| `drawSegmentedArc()` | 分段圆弧（4 色：绿→黄→橙→红） |
| `drawTextWithShadow()` | 文字阴影 |
| `drawBolt()` | 六角螺栓/螺母 |

## 各控件渲染特性

### TankWidget（罐子）
- 11 层绘制：阴影 → 金属框 → 螺栓(×12) → 井道渐变 → 刻度尺 → 液体(5 段渐变) → 玻璃反射 → 水面波纹(sin 动画，50ms) → 设定点三角标+"SP" → LED → 标签文字阴影

### PumpWidget（离心泵）
- 呼吸发光 `m_glowPhase` sin 脉动
- 入口/出口短管 + 金属环壳体
- 后弯离心式叶轮（QPainterPath::cubicTo 贝塞尔曲线）
- 速度弧：4 色分段，角度=0°~speed%

### ValveWidget（阀门）
- Q_PROPERTY + QPropertyAnimation (400ms OutCubic) 平滑开度过渡
- 金属阀体 + 法兰板 + 螺栓
- 阀杆旋钮 + 十字线
- 填充弧指示器 (0°~90°)

### PipeWidget（管道）
- 5 段垂直渐变 3D 圆柱体
- 端部法兰
- 7 颗粒子流动动画 (60ms QTimer, 高斯 α 分布)

### ValueDisplay（数值显示）
- LCD 暗蓝灰渐变面板 + 内凹阴影
- 报警发光脉动 (700ms)
- Consolas 等宽字体，值变化闪白 (180ms single-shot)

## 构建验证

```bash
cd stage8_ui_enhance
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
cmake --build build
cd build && windeployqt Stage8_UI_Enhance.exe
./Stage8_UI_Enhance.exe
```

---

*Status: Compiled & Running | Qt6::Widgets | No PLC*
