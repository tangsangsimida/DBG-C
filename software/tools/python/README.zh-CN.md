# DBG-C Python 开发工具

本目录集中存放供软件开发、检查和构建使用的 Python 工具。命令从仓库根目录执行；脚本按仓库内容定位路径，不依赖开发者的个人安装目录。要求 Python 3.10 或更新版本；格式化还需 clang-format 11 或更新版本。

## 源码格式化

格式化所有项目自有 C/C++ 文件（默认写入）：

```fish
python3 software/tools/python/format_code.py --all
```

只检查、不修改：

```fish
python3 software/tools/python/format_code.py --check --all
```

使用 `--c`、`--cpp` 或 `--all` 选择文件类型；`--dirs` 限定 `software/` 下的目录，`--workers` 设置并行数，`--verbose` 显示未变化文件，`--report PATH` 保存报告。也可将文件路径作为参数。clang-format 默认通过 `PATH` 查找，也可设置 `CLANG_FORMAT` 或用 `--clang-format` 指定命令。写入模式缓存位于 `software/build/.cache/`。第三方、生成、构建目录以及复制的 WCH CH585 头文件会被排除。

## Doxygen 注释覆盖率

```fish
python3 software/tools/python/check_comment_coverage.py software/common/packet_queue/src/dbgc_packet_queue.c
```

默认阈值为 100%。该工具采用启发式方式识别单行函数定义及相邻的 `/** ... */` 注释；多行签名和复杂声明属性需要人工复核。

## 明确编码转换

先预览已确认编码的单个文件：

```fish
python3 software/tools/python/convert_to_utf8.py path/to/source.c --from-encoding gb18030 --dry-run
```

确认预览后再加 `--write` 写入。工具不会猜测编码，并拒绝修改第三方、厂商、构建和生成目录。

## PoC 构建

```fish
python3 software/tools/python/build_poc1.py
```

构建工具通过 `PATH` 查找 `python3`、`git`、`cmake`、所选 CMake 构建器（如 GNU Make 或 Ninja）、主机 C 编译器及 `riscv-wch-elf-*` 工具。fish 用户可用 `fish_add_path /实际工具目录` 将程序目录加入 `PATH`；也可用 `set -gx DBGC_WCH_TOOLCHAIN_ROOT /实际工具链bin目录` 设置工具链目录。主机编译器可通过 `CC` 指定，CMake 命令可通过 `CMAKE` 指定，WCH 工具链也可通过 `DBGC_WCH_TOOLCHAIN_ROOT` 指定其 `bin` 目录。默认输出目录为 `software/build/poc1-ch585-threadx/`；`--build-dir NAME` 将输出放到 `software/build/NAME/`，构建产物和证据日志集中保存在 `software/build/`。为兼容既有复现命令，`--build-dir build/NAME` 也映射到 `software/build/NAME/`。示例：

```fish
python3 software/tools/python/build_poc1.py --build-dir host-review --host-cc cc
```

该脚本运行已有主机检查、交叉编译检查并记录构建证据；目前主机检查及目标对象检查的测试声明仍在构建脚本中显式维护，新增用例时需登记一次，但每次运行会自动执行。它不验证 CH585M 实板运行。构建目录、生成文件和环境配置均由开发者本地管理，不在脚本中设置机器专属安装路径。
