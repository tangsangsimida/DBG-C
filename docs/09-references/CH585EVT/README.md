# CH585EVT Reference Package

This directory is the single local holding place for the user-supplied `CH585EVT.ZIP` archive. The archive's `EVT/CH585_List_EN.txt` identifies Nanjing Qinheng Microelectronics Co., Ltd. and date `2026.08`.

## Use

Keep the archive intact. Extract only the specific official headers, sources, or libraries needed for an implemented driver, and place those files with that driver. Preserve accompanying vendor and third-party notices. The project does not yet contain CH585 driver code from which to select specific package components.

The archive's English and Chinese contents indexes are `EVT/CH585_List_EN.txt` and `EVT/CH585_List.txt`.

## Archive Integrity

SHA-256 of the supplied archive:

```text
cdab364ffc24d793b300f22b3aa7ddfd97306f880d3962448da47ef9da870274  CH585EVT.ZIP
```

The repository's `.gitignore` excludes files named `CH585EVT.ZIP`. The archive remains in the local workspace at this path but is not included in Git unless that ignore rule is changed.
