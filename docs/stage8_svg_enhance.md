# Stage8 — SVG + QPainter 混合渲染测试

SVG 静态底图 + QPainter 动态叠加的混合渲染方案，对标纯 QPainter 方案进行对比。

## 文件结构

```
stage8_svg_enhance/
├── CMakeLists.txt              Qt6::Widgets + Qt6::Svg
├── main.cpp
├── testwindow.h/.cpp           1050×700
│
├── svg_assets.h                6 段嵌入式 SVG 字符串
├── svg_renderer_pool.h/.cpp    QSvgRenderer 缓存池（单例，裸指针）
├── tank_widget.h/.cpp          SVG Tank 底图 + QPainter 液位/动画
├── pump_widget.h/.cpp          SVG Pump 底图 + QPainter 叶轮旋转
├── valve_widget.h/.cpp         SVG Valve 底图 + QPainter 开度指示
├── pipe_widget.h/.cpp          SVG Pipe 底图 + QPainter 流动粒子
└── value_display.h/.cpp        SVG Panel 底图 + QPainter 数值
```

## 6 段内嵌 SVG 素材

| 常量名 | 尺寸 | 内容 |
|--------|------|------|
| TANK_BODY | 200×340 | 罐体渐变 + 观察窗 + 铆钉列 + 顶部法兰 + 底部排水口 + 底部支撑 |
| PUMP_BODY | 180×160 | 蜗壳 + 电机壳体 + 入口/出口法兰 + 散热筋 + 铭牌 |
| VALVE_BODY | 140×160 | 手轮 + 阀杆 + 阀盖 + 填料函 + 法兰螺栓圈 |
| PIPE_HORIZ | 300×40 | 3D 管体 + 法兰 + 焊缝 |
| PIPE_VERT | 40×300 | 同上，垂直方向 |
| VALUE_PANEL | 200×120 | 外框 + 斜角 + LCD 面板 + 玻璃反光 |

## 关键实现细节

### SvgRendererPool（单例）

```cpp
class SvgRendererPool {
public:
    static SvgRendererPool *instance();
    QSvgRenderer *get(const QString &svgContent);  // 缓存命中 / 按需创建
    void clear();

private:
    QHash<QString, QSvgRenderer *> m_cache;  // 裸指针（unique_ptr 不可放入 QHash）
    ~SvgRendererPool() { qDeleteAll(m_cache); }
};
```

### SVG 字符串定界符
使用 `R"SVG_(...)SVG_"` 自定义定界符，避免 SVG 属性中 `)"` 与 `R"(...)"` 的标准 raw string 冲突。

## 遇到的问题与修复

1. **SVG 内 `)"` 截断 raw string**: 全部 6 段改用 `R"SVG_(...)SVG_"` 定界符。
2. **QHash + unique_ptr 不可拷贝**: 改用裸指针 + `qDeleteAll()`。

## 与纯 QPainter 方案对比

| 维度 | 纯 QPainter | SVG + QPainter |
|------|------------|----------------|
| 渲染性能 | 优（无 XML 解析） | 中（SVG 解析一次，缓存复用） |
| 视觉细节 | 需手绘 | SVG 更精细（铆钉、铭牌文字等） |
| 可维护性 | 代码量大 | SVG 可外部设计工具编辑 |
| 灵活性 | 高（随意变形） | 中（仅可叠 QPainter 层） |

---

*Status: Compiled & Running | Qt6::Widgets + Qt6::Svg | No PLC*
