# Electron Demo 运行记录

## 背景

在评估 Qt 替代方案时，需要运行 Electron 官方 quick-start demo 对比体验。项目地址：`d:/qt_learning/code/reference_projects/electron-quick-start/`

## 环境信息

- **操作系统**: Windows 11
- **Node.js**: v24.15.0
- **Electron 版本**: 33.4.0（最终可用版本）
- **代理**: Clash Verge @ 127.0.0.1:7897

## 踩坑过程

### 坑1：GitHub 下载被墙

Electron npm 包的 `postinstall` 脚本会自动从 GitHub Releases 下载 Electron 二进制（~110MB 的 zip）。

**现象**：
```
Downloading Electron binary...
TypeError: fetch failed
```

**原因**：GitHub 在国内被墙，直连下载失败。

**尝试的解决方案**：

| 方案 | 结果 |
|------|------|
| npm 设代理 `npm config set proxy` | 无效——Electron 的 `@electron/get` 使用环境变量 `HTTPS_PROXY`，不走 npm 配置 |
| 设置 `ELECTRON_MIRROR=https://npmmirror.com/mirrors/electron/` | 能下载，但二进制**有问题**（见坑2） |
| 用 curl + 代理直接下载官方 zip | **可行** |

**最终方案**——curl 走代理直连 GitHub：

```bash
curl -x http://127.0.0.1:7897 -L \
  -o /tmp/electron-v33.4.0-win32-x64.zip \
  "https://github.com/electron/electron/releases/download/v33.4.0/electron-v33.4.0-win32-x64.zip"
```

然后用 `npm install electron@33.4.0 --save-dev --ignore-scripts` 跳过自动下载，手动解压到 `node_modules/electron/dist/`。

### 坑2：npmmirror 镜像二进制不完整

通过 `ELECTRON_MIRROR` 从 npmmirror.com 下载的二进制存在以下问题：

- `electron.exe --version` 输出 `v24.15.0`（Node 版本号），而非 `v33.4.0`
- 缺少 `electron` 内置模块注册
- 启动时报 `Cannot read properties of undefined (reading 'whenReady')`

**结论**：不要用镜像站的 Electron 二进制，必须从 GitHub 官方下载。

### 坑3：ELECTRON_RUN_AS_NODE=1（最终根因）

即使换成了官方二进制，启动仍然失败，`process.type` 为 `undefined`，`require('electron')` 返回文件路径字符串。

**诊断过程**：

```bash
# 检查 Electron 环境
./electron.exe -e "console.log('process.type:', process.type)"
# 输出: process.type: undefined  ← 正常应为 "browser"

# 检查环境变量
./electron.exe -e "console.log('ELECTRON_RUN_AS_NODE:', process.env.ELECTRON_RUN_AS_NODE)"
# 输出: ELECTRON_RUN_AS_NODE: 1  ← 这就是根因！
```

**原因**：`ELECTRON_RUN_AS_NODE=1` 是 Electron 的内部机制——当该环境变量设为 `1` 时，Electron 退化为普通 Node.js 运行时，不会初始化 Chromium 浏览器进程，不会设置 `process.type`，也不会注册 `electron` 内置模块。

这个变量可能是系统环境变量、Shell 配置文件或 VSCode 终端继承的设置。

**解决方案**：

```bash
unset ELECTRON_RUN_AS_NODE && npx electron .
```

或直接运行二进制：

```bash
unset ELECTRON_RUN_AS_NODE && ./node_modules/electron/dist/electron.exe .
```

## 可用的启动命令

```bash
# 进入项目目录
cd d:/qt_learning/code/reference_projects/electron-quick-start

# 启动（务必 unset 那个环境变量）
unset ELECTRON_RUN_AS_NODE && npx electron .
```

## 关键文件

| 文件 | 作用 |
|------|------|
| `main.js` | Electron 主进程入口 |
| `index.html` | 渲染进程的 HTML 页面 |
| `preload.js` | 预加载脚本（沙箱安全） |
| `node_modules/electron/dist/electron.exe` | Electron 二进制（~188MB） |

## 与 Qt 的对比

| 维度 | Qt6 (C++) | Electron |
|------|-----------|----------|
| 二进制大小 | ~10MB（静态链接后 ~30MB） | ~188MB（仅 Electron 本体） |
| 启动速度 | 毫秒级 | 1-3 秒 |
| 内存占用 | ~20MB | ~100MB+ |
| 开发语言 | C++ | JavaScript/HTML/CSS |
| UI 渲染 | QPainter / QSS | Chromium |
| 工业协议支持 | Snap7（原生 C 库） | Node.js 通过 node-snap7 |
| 实时性能 | 优秀 | 受限于 V8 GC |
| 适用场景 | 工业 HMI、嵌入式、实时控制 | 跨平台桌面应用（VS Code、Slack 等） |

**对于工业 HMI 项目，Qt 依然是首选**——原生性能、成熟的工业控件生态、以及直接的 C/C++ 硬件通信能力是 Electron 无法比拟的。

## 补充：如何检查环境变量

```bash
# 在 Git Bash / MSYS2 中
echo $ELECTRON_RUN_AS_NODE

# 在 PowerShell 中
$env:ELECTRON_RUN_AS_NODE

# 如果输出 "1"，说明需要 unset
```
