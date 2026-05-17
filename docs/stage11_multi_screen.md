# Stage11 — 多屏导航框架

工业 HMI 多画面切换框架。QPainter 自绘导航栏 + QStackedWidget 画面管理 + 画面生命周期。

## 文件结构

```
stage11_multi_screen/
├── CMakeLists.txt              Qt6::Widgets
├── main.cpp
│
├── hmi_screen.h                画面基类
├── nav_bar.h/.cpp              自绘导航栏（48px）
├── sim_data_manager.h/.cpp     共享数据源（6 标签正弦波）
├── testwindow.h/.cpp           主框架（NavBar + QStackedWidget + 全局 Timer）
│
├── screen_overview.h/.cpp      总览仪表盘（3×2 自绘卡片）
├── screen_process.h/.cpp       流程画面（Tank → Pump → Valve）
├── screen_alarm.h/.cpp         报警列表（简化阈值检测）
├── screen_trend.h/.cpp         趋势曲线（300 点环形缓冲）
├── screen_settings.h/.cpp      系统信息（QTableWidget + 状态）
│
└── (process_widgets 文件)      tank / pump / valve / pipe / value_display
```

## 核心接口

### HmiScreen（画面基类）

```cpp
class HmiScreen : public QWidget {
    Q_OBJECT
public:
    explicit HmiScreen(QWidget *parent = nullptr);
    virtual void onEnter();   // 进入画面
    virtual void onLeave();   // 离开画面
    virtual void onTick();    // 100ms 定时更新（仅活动画面）
    void setSimData(SimDataManager *sim);
protected:
    SimDataManager *m_simData = nullptr;
};
```

### NavBar（自绘导航栏）

- 48px 固定高度
- 按钮 ~96px 宽，活跃状态：`#2a5a8a` 填充 + 顶部 `#4a9aff` 强调线
- 右侧 Consolas 时钟 `yyyy-MM-dd HH:mm:ss` （1 秒刷新）
- 脉动红色报警圆点（`m_alarmUnacked > 0` 时显示数量）
- `addButton(label, icon)` / `setActiveIndex(int)` / `setAlarmCount(active, unacked)`

### TestWindow 画面调度

```cpp
void TestWindow::addScreen(HmiScreen *screen, const QString &label, const QString &icon);
void TestWindow::activateScreen(int index);  // 旧画面 onLeave() → 切换 → 新画面 onEnter()
void TestWindow::onGlobalTick();             // SimDataManager::tick() → activeScreen::onTick()
```

## 5 个演示画面

| 画面 | 内容 | 数据绑定 |
|------|------|---------|
| **总览** | 3×2 OverviewCard 卡片网格（标签名+值+单位+绿/橙/红状态条） | `valueChanged` 信号 |
| **流程** | Tank→Pipe→Pump→Pipe→Valve 流向图 + 6 ValueDisplay | `onTick()` 轮询 |
| **报警** | 硬编码阈值（复刻 stage9）+ 闪烁 + 确认按钮 | `onTick()` 检测 |
| **趋势** | 3 曲线 (TEMP/PRESS/TANK)，300 点缓冲，30 秒窗口 | `onTick()` 追加 |
| **设置** | QTableWidget 6 标签实时值 + 系统信息 | `onTick()` 刷新 |

---

*Status: Compiled & Running | Qt6::Widgets | No PLC*
