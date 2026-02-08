# RISC-V AudioMark Challenge: Vectorized Q15 AXPY

**Author:** Shreyas Singh  
**Date:** February 2026  
**Target:** RISC-V Vector Extension (RVV) v1.0

## Overview
This repository contains a vectorized implementation of the `q15_axpy` function (Alpha * X + Y) for the EEMBC AudioMark benchmark. The solution is written in C using RISC-V Vector (RVV) intrinsics to achieve high-performance DSP processing on RISC-V hardware.

## Design Choices

### 1. Vector-Length Agnostic (VLA)
The implementation utilizes the `vsetvl` intrinsic to dynamically query the hardware's vector length (VLEN) at runtime.
* **Benefit:** The code is portable across all RVV-compliant processors (from embedded cores with VLEN=128 to high-performance cores with VLEN=512) without recompilation.

### 2. Hardware Saturation
Audio processing requires Q15 fixed-point arithmetic with saturation. Instead of costly software branching (e.g., `if (x > MAX) ...`), this solution uses hardware-native saturating instructions:
* `vsmul.vx`: Saturating multiply (Vector-Scalar).
* `vsadd.vv`: Saturating add (Vector-Vector).
* **Benefit:** Eliminates branch misprediction penalties and maximizes pipeline efficiency.

### 3. Register Grouping (LMUL=8)
The vector configuration is set to `m8` (LMUL=8), grouping 8 vector registers for each operation.
* **Benefit:** Maximizes the number of elements processed per instruction, amortizing the loop overhead (pointer updates, counters) over a larger batch of data.

## Performance Analysis (Theoretical)

Compared to a standard scalar C implementation, the vectorized approach offers significant speedup:

| Metric | Scalar Implementation | Vector Implementation (VLEN=128) |
| :--- | :--- | :--- |
| **Ops per Cycle** | 1 (Best Case) | ~8 (Parallel) |
| **Saturation Cost** | High (Branching) | Zero (Hardware) |
| **Throughput** | ~1 element / 10-15 cycles | ~1 element / 1 cycle |
| **Est. Speedup** | 1x | **8x - 12x** |

## Build & Verification

This code was verified using the **Godbolt Compiler Explorer** environment with the following configuration:
* **Compiler:** RISC-V GCC (rv32gcv / rv64gcv)
* **Simulator:** Spike (ISA Simulator)
* **Verification:** Verified via assembly inspection for correct instruction generation (`vsetvl`, `vsmul`, `vsadd`).

### Building (Requires RISC-V Toolchain)
To compile the solution locally:
```bash
riscv64-unknown-elf-gcc -march=rv64gcv -O3 -c src/q15_axpy_rvv.c -o q15_axpy.o
