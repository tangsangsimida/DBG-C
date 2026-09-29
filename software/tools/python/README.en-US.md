# DBG-C Python Developer Tools

This directory contains Python tools for software development, checks, and builds. Run commands from the repository root. Scripts locate repository content relative to themselves and do not depend on a developer-specific installation directory. Python 3.10 or newer is required; formatting also requires clang-format 11 or newer.

## Source Formatting

Format all project-owned C/C++ files (write mode is the default):

```sh
python3 software/tools/python/format_code.py --all
```

Check without modifying files:

```sh
python3 software/tools/python/format_code.py --check --all
```

Use `--c`, `--cpp`, or `--all` to select file types. `--dirs` narrows scanning to directories under `software/`, `--workers` sets parallelism, `--verbose` lists unchanged files, and `--report PATH` writes a report. Explicit file paths are also supported. clang-format is resolved through `PATH` by default; set `CLANG_FORMAT` or pass `--clang-format` to select another command. Write-mode cache data is stored under `software/build/.cache/`. Third-party, generated, build, and copied WCH CH585 header paths are excluded.

## Doxygen Comment Coverage

```sh
python3 software/tools/python/check_comment_coverage.py software/common/packet_queue/src/dbgc_packet_queue.c
```

The default threshold is 100%. The checker heuristically recognizes single-line function definitions and adjacent `/** ... */` comments. Review multiline signatures and complex declaration attributes manually.

## Explicit Encoding Conversion

Preview one file whose source encoding has been confirmed:

```sh
python3 software/tools/python/convert_to_utf8.py path/to/source.c --from-encoding gb18030 --dry-run
```

After confirming the preview, add `--write` to modify the file. The converter never guesses encodings and rejects third-party, vendor, build, and generated paths.

## PoC Build

```sh
python3 software/tools/python/build_poc1.py
```

The build tool resolves `python3`, `git`, `cmake`, the selected CMake generator (such as GNU Make or Ninja), the host C compiler, and `riscv-wch-elf-*` tools through `PATH`. Add the executable directories actually used to `PATH`; set `CC` to select the host compiler, `CMAKE` to select the CMake command, or `DBGC_WCH_TOOLCHAIN_ROOT` to identify the WCH toolchain `bin` directory. Select a build directory with `--build-dir`; it must remain under the PoC directory. For example:

```sh
python3 software/tools/python/build_poc1.py --build-dir build/host-review --host-cc cc
```

The script runs the existing host checks and target-object checks, then records build evidence. Their test declarations are still explicit in the build script, so a new case must be registered once; each run executes the registered checks automatically. The script does not verify CH585M board runtime. Build outputs and local environment configuration remain developer-managed; the scripts do not set machine-specific installation paths.
