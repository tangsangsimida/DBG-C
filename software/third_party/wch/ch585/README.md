# WCH CH585 EVT UID Support

This directory contains the minimum WCH material required to link the CH585
UID-read adapter:

- `ISP585.h`, extracted from `EVT/EXAM/SRC/StdPeriphDriver/inc/ISP585.h` in the
  supplied `CH585EVT.ZIP` archive.
- `libISP585.a`, extracted from
  `EVT/EXAM/SRC/StdPeriphDriver/libISP585.a` in that archive.

The EVT archive index is dated 2026.08. The source file header identifies WCH
StdPeriphDriver V1.2 dated 2021/11/17. The library is linked only for CH585
firmware. SHA-256 values are:

- `ISP585.h`: `244b966e79381b7ebf47c0a4942f001ee066c2cc9d32538cf7d4296984c81db8`
- `libISP585.a`: `8ede32a24e09344e86ecd74731069e19d6befa287bd6de2dd7c757c6f1d06a9e`

The upstream `GET_UNIQUE_ID()` wrapper returns `void` and ignores the status
from `FLASH_EEPROM_CMD()`. DBG-C calls the documented lower-level command
directly so it can report ROM-command failure to its caller. The UID bytes and
ROM command behavior still require CH585M board verification.
