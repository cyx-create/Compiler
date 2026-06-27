# FDMJ 完整编译器

本目录为课程最终提交的**代码包根目录**，包含 Makefile、README 与 `test/` 目录。

## 环境要求 (a)

- 64 位 Ubuntu 20.04 或更高版本 (x86)
- 课堂所需工具：`cmake`、`ninja`、`flex`、`bison`、`make`、`g++`
- 运行 ARM 程序：`arm-linux-gnueabihf-gcc`、`qemu-arm`

## 快速开始 (c)

```bash
cd Compiler
make build          # 构建编译器 fdmjc
make compile        # 编译 test/ 下每个 .fmj，生成各阶段中间文件与 6 种 .colored.s
make run            # QEMU 运行 base（无优化）汇编
make run-const      # 常量传播 (SCCP)
make run-loop1      # 循环不变式外提
make run-loop2      # 归纳变量 / 强度削弱
make run-allloop    # 上述两种循环优化
make run-allopt     # 全部优化
```

## Make 目标说明

| 命令 | 作用 |
|------|------|
| `make build` | 同步 HW/PARSING 源码并构建 `build/tools/main/fdmjc` |
| `make compile` | 对 `test/*.fmj` 逐个完整编译（6 种优化级别） |
| `make run` | 运行 `*.base.colored.s`（无优化） |
| `make run-const` | 运行 `*.const.colored.s` |
| `make run-loop1` | 运行 `*.loop1.colored.s` |
| `make run-loop2` | 运行 `*.loop2.colored.s` |
| `make run-allloop` | 运行 `*.allloop.colored.s` |
| `make run-allopt` | 运行 `*.allopt.colored.s` |
| `make clean` | 清理 build 与 test 下生成的中间文件 |

## 测试集

`test/` 目录含 **46 个** `.fmj` 源文件（由 `scripts/setup_test.sh` 从 HW3/HW5/HW12 安装）。

| 类别 | 用例 |
|------|------|
| IR | `irtest1`–`irtest22` |
| Quad | `quadtest1`–`9`, `simplecall1/2` |
| 后端 | `insttest0`–`4`, `fibonacci`, `bubblesort`, `optloopivtest1`–`6` |

## 输出文件 (d)

对 `test/<name>.fmj` 执行 `make compile` 后，典型产物包括：

| 文件 | 阶段 |
|------|------|
| `<name>.src` | 源程序 listing（去注释） |
| `<name>.2-semant.ast` | 带语义信息的 AST (XML) |
| `<name>.3.irp` / `<name>.3-canon.irp` | IR 树 |
| `<name>.4.quad` / `<name>.4-xml.quad` | Quad |
| `<name>.4-ssa.quad` / `<name>.4-ssa-withflow-xml.quad` | SSA 及流图 |
| `<name>.4-ssa-opt.quad` 等 | 各优化级别 Quad |
| `<name>.<opt>.5-xml.asm` / `<name>.<opt>.s` | 指令选择与调度后的汇编 |
| `<name>.<opt>.colored.s` | 寄存器分配后的 ARM 汇编（QEMU 运行） |

优化后缀 `<opt>`：`base`, `const`, `loop1`, `loop2`, `allloop`, `allopt`。

## 编译流水线

```
.fmj → PARSING parser → 语义分析 → IR → Quad → SSA → 优化 → 指令选择/调度 → 寄存器分配 → .colored.s
```

后端测试（insttest 等）若存在 HW11/HW12 bootstrap SSA 文件，可跳过前端直接从 SSA _codegen。

## 目录结构 (b)

```
Compiler/
  Makefile
  README.md
  test/                 # 所有 .fmj 测试源文件
  include/ lib/         # 整合后的编译器源码
  lib/frontend/         # PARSING 提供的 lexer/parser
  lib/pipeline/         # 端到端流水线
  tools/main/fdmjc      # 主编译器
  vendor/libsysy/       # SysY 运行时 (ARM)
  scripts/              # 同步、测试、验证脚本
```

## 手动编译单个程序

```bash
./build/tools/main/fdmjc test irtest1 all      # 6 种优化
./build/tools/main/fdmjc test quadtest3 base   # 仅 base
./build/tools/main/fdmjc test quadtest3 quad-only
```
