# DBG-C Python Developer Tools

These scripts are adapted for the C firmware tree under `software/`. They reuse the scoped formatting, comment-coverage reporting, and encoding-conversion workflows used by sibling projects. All file paths are resolved relative to this `software/` root.

The commands work from the repository root. They also work from `software/` after omitting the `software/` prefix. The scripts require Python 3.9 or newer; formatting also requires clang-format 11 or newer.

## Formatting

Format all project-owned C/C++ files (write mode is the default):

```sh
python3 software/tools/python/format_code.py --all
```

Check all project-owned C/C++ files without modifying them:

```sh
python3 software/tools/python/format_code.py --check --all
```

Format only explicitly named project-owned files:

```sh
python3 software/tools/python/format_code.py software/common/packet_queue/src/dbgc_packet_queue.c
```

The formatter uses `software/.clang-format`. `third_party/`, generated/build trees, and the copied WCH CH585 headers are excluded. Assembly is not passed to clang-format.

## Doxygen Coverage Report

```sh
python3 tools/python/check_comment_coverage.py common/packet_queue/src/dbgc_packet_queue.c
```

The default threshold is 100%. The checker is heuristic: it reports single-line function definitions and adjacent `/** ... */` comments. It can miss multiline signatures or complex declaration attributes; inspect changed files directly and do not treat this report as a complete parser result.

## Explicit-Encoding Conversion

Preview one confirmed legacy text encoding:

```sh
python3 tools/python/convert_to_utf8.py path/to/source.c --from-encoding gb18030 --dry-run
```

Write only after the preview confirms the expected source encoding and result:

```sh
python3 tools/python/convert_to_utf8.py path/to/source.c --from-encoding gb18030 --write
```

The converter never guesses an encoding, handles one file per invocation, and rejects vendor, third-party, build, and generated paths.
