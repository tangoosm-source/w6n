# w6n

ARM64 (AArch64) ELF analyzer and roundtrip verifier.
Capstone disassembler + own ARM64 assembler (OpcodeByte).

## Features

- **ELF loading** — .text section, symbols, entry point
- **Disassembly** — Capstone 5.0.7, ARM64
- **Roundtrip verification** — Capstone vs OpcodeByte, byte-level
- **Coverage** — 99.65% on `/usr/bin/ls`, FARK: 0

## Supported instructions

- **Integer ISA**: mov, add, sub, cmp, and, orr, eor, lsl, lsr, asr, ror,
  mul, madd, udiv, sdiv, umulh, smulh, ldr/str (all addressing), ldp/stp,
  cbz, cbnz, tbz, tbnz, b, bl, b.cond, adr, adrp, csel, cset, ccmp, ccmn,
  ubfx, sbfx, ubfiz, bfi, extr, clz, rbit, rev, mrs, msr, dmb, dsb, isb,
  bti, paciasp, autiasp, fmov, fadd, fsub, fmul, fdiv, fabs, fsqrt, fcmp,
  add v, cmeq v, cmge v, and v, orr v, eor v, bsl, ldr q0, stp q0,q1 …

## Installation

### From PyPI (source distribution — builds locally)

    pip install w6n

### Build requirements

- Python 3.8+
- CMake, C++17 compiler (g++ or clang++)
- Capstone development headers
- pkg-config

### Termux (Android)

    pkg install cmake make clang pkg-config capstone
    pip install w6n

### Ubuntu (native or proot)

    sudo apt install build-essential cmake pkg-config libcapstone-dev
    pip install w6n

The wheel is platform-specific:
- Termux: `w6n-0.1.0-cp314-cp314-android_24_arm64_v8a.whl`
- Ubuntu: `w6n-0.1.0-cp314-cp314-linux_aarch64.whl`

## Usage

    asmanalyzer bytes 20 00 80 d2 c0 03 5f d6
    asmanalyzer elf binary.elf --disasm-all
    asmanalyzer elf binary.elf --functions
    asmanalyzer elf binary.elf --disasm _start
    asmanalyzer elf-roundtrip binary.elf

## Coverage

Termux `/system/bin/ls` (66.674 instructions):
- OK: 66.464 (99.69%)
- FARK: 0
- Unsupported: 210

Ubuntu `/usr/bin/ls` (1.057.064 instructions):
- OK: 1.053.358 (99.65%)
- FARK: 0
- Unsupported: 3.706

Unsupported: movi/mvni, pmull, ucvtf/scvtf, NEON varied (dup, tbl, ext, zip1, ushr, shrn, sri), element insert/extract.

## License

MIT
