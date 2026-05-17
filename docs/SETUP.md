# 新电脑 Qt 环境搭建指南

## 1. 安装 Qt

打开 Qt 在线安装器 https://www.qt.io/download-qt-installer

安装时勾选：
- **Qt 6.11.0** → `MinGW 64-bit`（或者你实际安装的版本）
- **Qt Tools** → `CMake`、`Ninja`、`MinGW`（安装器会一起装）

安装路径默认 `C:\Qt`

## 2. 安装 Git

winget install --id Git.Git

或者从 https://git-scm.com/download/win 下载

## 3. 安装 VS Code + 插件

从 https://code.visualstudio.com/ 下载安装，然后安装插件：
- **C/C++** (Microsoft)
- **CMake Tools** (Microsoft)

## 4. 安装 GitHub CLI

winget install --id GitHub.cli

## 5. 克隆项目

git clone https://github.com/PandaZhym/qt_process_hmi.git d:/qt_learning/code/src/qt_process_hmi

## 6. 构建运行

# 以 stage3 为例
cd d:/qt_learning/code/src/qt_process_hmi/siemens_plc_comm/stage3_read_data
export PATH="/c/Qt/Tools/CMake_64/bin:/c/Qt/Tools/mingw1310_64/bin:/c/Qt/Tools/Ninja:$PATH"
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH=C:/Qt/6.11.0/mingw_64
cmake --build build

## 7. Git 日常同步

### 新电脑首次拉取

git clone https://github.com/PandaZhym/qt_process_hmi.git

### 每次工作开始

git pull

### 每次工作结束

git add -A
git commit -m "做了什么改动"
git push

Token 已保存在 `~/.git-credentials`，不需要每次输入密码。