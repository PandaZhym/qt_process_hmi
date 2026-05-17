# 从 .cpp / .h 源码到 .exe 可执行文件 —— 小白上手指南

## 你需要什么

你的电脑需要安装过 **Qt**（带 MinGW 编译器）。只要装过 Qt 6.x，以下工具就都有了：

| 工具 | 位置 | 作用 |
|------|------|------|
| `g++.exe` | `C:\Qt\Tools\mingw1310_64\bin\` | C++ 编译器（把 .cpp 变成 .obj） |
| `ninja.exe` | `C:\Qt\Tools\Ninja\` | 构建工具（按规则调用 g++） |
| `cmake.exe` | `C:\Qt\Tools\CMake_64\bin\` | 构建系统生成器（根据 CMakeLists.txt 生成构建规则） |

> 版本号可能不同，比如 `mingwXXXX_64`，只要找对应的路径就行。

---

## 第一步：永久添加工具到 PATH

PATH 是系统查找命令的目录列表。把上面三个工具目录加进去，以后在任何终端里都能直接敲 `cmake`、`g++`、`ninja`。

1. 打开 **Windows 设置 → 系统 → 关于 → 高级系统设置 → 环境变量**
2. 在"用户变量"里找到 `Path`，双击编辑
3. 新增三条（根据你电脑上实际路径调整）：
   ```
   C:\Qt\Tools\CMake_64\bin
   C:\Qt\Tools\mingw1310_64\bin
   C:\Qt\Tools\Ninja
   ```
4. 全部确定保存
5. 打开一个新终端（cmd 或 PowerShell），分别输入 `cmake --version`、`g++ --version`、`ninja --version`，都能显示版本号说明配好了

---

## 第二步：理解你的项目文件

一个最简单的项目目录长这样：

```
my_project/
├── CMakeLists.txt     ← 你写：告诉 cmake "我有哪些 .cpp，想生成什么"
├── main.cpp           ← 你写：程序代码
├── snap7.h            ← 别人给的：函数声明
├── snap7.cpp          ← 别人给的：C++ 封装层实现
├── snap7.dll          ← 别人给的：运行时动态库（程序跑的时候需要）
└── libsnap7_mingw.a   ← 从 DLL 生成的：MinGW 导入库（链接时用）
```

**每个文件的角色：**

| 文件 | "如果把它比作..." | 谁提供 |
|------|-------------------|--------|
| `CMakeLists.txt` | 施工图纸，告诉工人怎么盖 | 你写 |
| `main.cpp` | 你写的程序逻辑 | 你写 |
| `snap7.h` | 产品的说明书目录（有哪些函数、怎么调用） | 第三方库 |
| `snap7.cpp` | C++ 的包装盒代码 | 第三方库 |
| `snap7.dll` | 真正的引擎（程序跑起来时调用它） | 第三方库 |
| `libsnap7_mingw.a` | 引擎的接口适配器（编译时用，告诉 exe"引擎在 dll 里"） | 你从 DLL 生成 |

---

## 第三步：编译四步走

在项目目录下打开终端（在文件夹地址栏输入 `cmd` 回车）。

### 3.1 配置（生成构建规则）

```bash
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++
```

每个参数的含义：

| 参数 | 含义 |
|------|------|
| `-B build` | 构建文件放到 `build` 子目录（不污染源码） |
| `-G "Ninja"` | 用 Ninja 作为构建工具（Qt 自带的，比 make 快） |
| `-DCMAKE_CXX_COMPILER=g++` | 指定用 g++ 编译器 |

这一步成功后会显示 `-- Build files have been written to: .../build`。

### 3.2 编译（真正的生成 exe）

```bash
cmake --build build
```

这一步调用 g++ 编译 .cpp → .obj，然后链接 .obj + .a → .exe。成功后会显示 `[2/2] Linking CXX executable ...`。

### 3.3 拷贝 DLL（exe 运行时需要）

```bash
copy snap7.dll build\
```

Windows 找 DLL 的规则是先从 exe 所在目录找，找不到才去 PATH 找。所以把 DLL 放到 exe 旁边最可靠。

### 3.4 运行

```bash
build\你的项目名.exe
```

或者在文件管理器中双击 `build\` 下的 exe。

---

## 常见问题

### Q: 编译报 `undefined reference to ...`
**原因**：链接器找不到函数实现。
**解决**：检查 CMakeLists.txt 里 `add_executable` 是否列出了所有 .cpp，`target_link_libraries` 是否正确写了 .a 文件名。

### Q: 运行时找不到 DLL
**原因**：exe 旁边没有 snap7.dll。
**解决**：把 snap7.dll 复制到 exe 所在目录。

### Q: 中文乱码
**原因**：Windows 控制台默认用 GBK 编码，源码是 UTF-8。
**解决**：在 `main()` 最开头加一行 `system("chcp 65001 > nul");`

### Q: 双击 exe 闪一下就没了
**原因**：控制台程序跑完自动关闭。
**解决**：在 `main()` 的 `return 0;` 前加一行 `system("pause");`

---

## CMakeLists.txt 模板

```cmake
cmake_minimum_required(VERSION 3.16)
project(我的项目名 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 列出所有 .cpp 文件
add_executable(${PROJECT_NAME}
    main.cpp
    snap7.cpp
)

# 静态链接（exe 不依赖 MinGW 运行时 DLL）
target_link_options(${PROJECT_NAME} PRIVATE -static)

# 链接需要的库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/libsnap7_mingw.a
    ws2_32
)
```

`${CMAKE_SOURCE_DIR}` 是一个变量，值 = CMakeLists.txt 所在的目录。写它而不是写死绝对路径，这样项目挪到别的目录也能编译。

---

## 术语速查

| 术语 | 一句话解释 |
|------|-----------|
| 编译 (compile) | 把 .cpp 源文件翻译成 .obj 目标文件 |
| 链接 (link) | 把多个 .obj + .a 拼成一个 .exe |
| 静态库 (.a / .lib) | 代码直接嵌入 exe，exe 变大但不依赖外部文件 |
| 动态库 (.dll) | 代码单独存放，exe 跑到那里时才加载 |
| 导入库 (.a / .lib) | 中介文件，告诉链接器"函数在某个 DLL 里" |
| 头文件 (.h) | 函数声明，告诉编译器"有这些函数可用" |
