# ImGui PLC Tool — 独立 PLC 变量监视/调试工具

## 背景

在完成 13 阶段 Qt SCADA 学习路线后，需要一个**独立于 Qt 的轻量 PLC 调试工具**。Dear ImGui 提供了极轻量的 GUI 方案（~5.6MB），搭配 Snap7 可以直接读写 PLC 所有变量，适合现场调试和快速验证。

项目地址：`d:/qt_learning/code/reference_projects/imgui_plc_tool/`

## 技术栈

| 层 | 技术 | 说明 |
|---|---|---|
| UI 渲染 | Dear ImGui + Win32 + OpenGL3 | 即时模式 GUI，零 Qt 依赖 |
| PLC 协议 | Snap7 `TS7Client` (C++ class) | 与 Qt 版本共用同一套 `snap7.dll`/`libsnap7_mingw.a` |
| 批量读取 | `ReadMultiVars` | 一次轮询读取全部变量，效率远高于逐条读取 |
| 单点写入 | `WriteArea` | 点击表格 Write 按钮，弹窗输入新值写入 PLC |
| 配置持久化 | nlohmann/json (header-only) | 导入/导出 JSON 配置文件 |

## 文件结构

```
imgui_plc_tool/
├── CMakeLists.txt           # 无 Qt，纯 Win32+OpenGL
├── main.cpp                 # ~600 行，全部 UI + 通讯逻辑
├── snap7.h / snap7.cpp      # Snap7 C++ wrapper
├── libsnap7_mingw.a         # 静态库 (98KB)
├── snap7.dll                # 动态库 (244KB)
├── json.hpp                 # nlohmann/json v3.11.3
├── imgui/                   # Dear ImGui 源码 + Win32/OpenGL3 后端
│   ├── imgui.h / imgui.cpp / imgui_draw.cpp / imgui_widgets.cpp / imgui_tables.cpp
│   ├── imgui_impl_win32.h / .cpp
│   └── imgui_impl_opengl3.h / .cpp
└── build/
    └── ImGuiPlcTool.exe     # 最终产物 ~5.6MB
```

## 功能

### 连接面板（左侧）

- IP / Rack / Slot 输入
- Connect / Disconnect 按钮（颜色区分状态）
- CPU 状态显示：**RUN**（绿色）/ **STOP**（黄色）/ Unknown（灰色）
- PDU 协商长度
- 轮询间隔设置（最低 100ms）+ Start / Stop Polling

### 变量表（右侧，9 列）

| 列 | 说明 |
|---|---|
| Name | 变量名（可点击选中） |
| Area | PE / PA / MK / DB / CT / TM |
| DB# | DB 块编号 |
| Start | 起始字节偏移 |
| Size | 读取字节数 (1-8) |
| Type | BOOL / INT8 / UINT8 / INT16 / UINT16 / INT32 / FLOAT32 |
| Value | 实时值（绿色=有效，红色=读取失败） |
| Valid | OK / ERR |
| 操作 | **Write** 按钮 + **X** 删除按钮 |

### 写入 PLC

点击 Write → 弹出模态窗口 → 输入新值 → 自动转换大端字节序 → `WriteArea`

支持所有 7 种类型的写回：BOOL (0/1/true/false/on/off)、整数、浮点。

### JSON 导入/导出

```
File → Import Tags JSON...  → 加载 .json（包含 tags + 连接参数）
File → Export Tags JSON...  → 保存当前配置到 .json
```

JSON 格式示例：
```json
{
  "rack": 0,
  "slot": 1,
  "poll_interval": 500,
  "tags": [
    {"name": "Motor1_Run",  "area": 3, "db": 1, "start": 0, "size": 2, "valType": 4},
    {"name": "Tank_Temp",   "area": 3, "db": 1, "start": 2, "size": 4, "valType": 6},
    {"name": "Valve_Open",  "area": 3, "db": 1, "start": 6, "size": 1, "valType": 0}
  ]
}
```

## 数据类型与字节序

S7-300/400/1200/1500 全部使用**大端字节序 (Big Endian)**：

| valType | 类型 | 字节数 | 示例 |
|---------|------|--------|------|
| 0 | BOOL | 1 | 0x01 = TRUE |
| 1 | INT8 | 1 | 0xFF = -1 |
| 2 | UINT8 | 1 | 0x64 = 100 |
| 3 | INT16 | 2 | 0x00 0x64 = 100 |
| 4 | UINT16 | 2 | 0x03 0xE8 = 1000 |
| 5 | INT32 | 4 | 大端 32 位有符号 |
| 6 | FLOAT32 | 4 | 大端 IEEE 754 单精度 |

## 构建

```bash
cd d:/qt_learning/code/reference_projects/imgui_plc_tool
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./ImGuiPlcTool.exe
```

## Qt 版本 vs ImGui 版本对比

| 维度 | Qt Stage7 (C++) | ImGui PLC Tool |
|------|-----------------|----------------|
| 二进制大小 | ~30MB+ (windeployqt) | ~5.6MB |
| 启动速度 | ~1-2 秒 | 毫秒级 |
| UI 框架 | QMainWindow + QWidget + QSS | 即时模式 ImGui |
| 线程模型 | QThread + moveToThread | 单线程主循环轮询 |
| 代码量 | ~8 文件分散 | ~600 行单文件 |
| 适用场景 | 生产 SCADA 框架 | 现场调试、变量监视 |
| 工业控件 | TankWidget, PipeWidget 等 | 纯表格 + 弹窗 |
| PLC 通讯 | 同一套 Snap7 | 同一套 Snap7 |

**结论：ImGui 版本是 Qt SCADA 框架的轻量补充**——调试、快速验证、变量监视用 ImGui，生产运行用 Qt。

## 踩坑记录

无。这次没有踩坑，因为：
- Snap7 库和头文件直接复用 stage6/stage7 已验证版本
- ImGui Win32+OpenGL3 样例之前已跑通
- 唯一编译问题是 `dwmapi` 链接缺失（ImgUI win32 后端需要）和 `ImGuiChildFlags_Borders` 拼写