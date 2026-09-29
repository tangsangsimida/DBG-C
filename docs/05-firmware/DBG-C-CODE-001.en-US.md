# DBG-C Coding Standard

**Document ID:** DBG-C-CODE-001　**Version:** V0.1　**Status:** Draft

## 1. Scope

This standard applies to DBG-C-maintained C, C++, and assembly code under `software/`. Formatting is governed by [`software/.clang-format`](../../software/.clang-format). The Linux-kernel-based format and project requirements use the 600C software project's `.clang-format` and `AGENTS.md` as their baseline, adapted to DBG-C's languages and directory structure.

Do not mass-format ThreadX, CMSIS-DAP, WCH EVT, other third-party, or generated files. Follow their upstream style and license requirements when changing them. New DBG-C adaptation code follows this standard.

## 2. Formatting and Naming

- Use tabs with an 8-column indentation width; target 100 columns.
- Use K&R braces for control statements and put function opening braces on a new line. Put C++ class opening braces on a new line.
- Place pointer asterisks next to variable names. Do not visually align declarations, assignments, or trailing comments.
- Do not automatically reorder includes. Preserve the established order: module header, standard library, vendor/RTOS, then project headers.
- Name new project files and symbols in lowercase `snake_case` with clear module prefixes; continue the existing `dbgc_<subsystem>_<purpose>` pattern. Do not rename existing public interfaces only to satisfy style.
- Use `clang-format` for DBG-C-owned C/C++ files; the configuration requires clang-format 11 or newer.

## 3. Comments and Doxygen

- Write new or modified project-code comments as Chinese Doxygen block comments in `/** ... */` form; do not use `///`. Place comments before the complete declaration, definition, or statement. Never insert them inside expressions, parameter lists, initializer lists, or macro expressions.
- Every logical function in each modified project-owned C/C++ source file must have a Doxygen comment. Measure coverage across the entire modified file, not only added functions. Describe purpose and, where relevant, parameters, return values, state changes, resource constraints, or interrupt requirements.
- Document public interfaces, structures, enumerations, and important macros with their purpose, values, or calling constraints. Avoid comments that merely repeat obvious local statements; explain intent, constraints, or non-obvious behavior.
- Do not rewrite untouched third-party, RTOS, generated, or original EVT files to satisfy comment coverage. When modification is required, limit it to necessary code and preserve provenance and change boundaries.

## 4. Changes and Verification

Format only project-owned code in the change; avoid unrelated diffs. Formatting and comment tools are under [`software/tools/python/`](../../software/tools/python/README.en-US.md); see its README for commands and scan boundaries. The comment-coverage tool is heuristic and does not replace source review. Run applicable builds or host checks documented by the module README and `DBG-C-FW-001`, and report board-validation status separately. A successful compile does not establish hardware behavior. For interface or controlled-document changes, cross-check both language files, callers, and related requirements/test records.
