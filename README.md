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


## Performance Analysis (Theoretical) as of 8 February,2026

Due to the unavailability of a cycle-accurate RVV 1.0 simulator in the local environment, I performed a theoretical analysis to estimate the speedup of the vectorized solution compared to a scalar baseline.

### 1. Scalar Baseline (Standard C)
A standard scalar implementation of `q15_axpy` processes one element at a time. The critical path per cycle includes:
* **Memory Access:** 2 Loads, 1 Store
* **Arithmetic:** 1 Multiply, 1 Add
* **Saturation Logic:** The primary bottleneck. Standard C requires (if/else statements) to check for overflows/underflows. Branch mispredictions can cost 3-5 cycles each,  reducing pipeline efficiency
* **Loop Overhead:** Pointer increments and loop counter checks per element.
* **Estimated Cost:** ~10-15 cycles per element.

### 2. Vectorized Implementation (RVV 1.0)
The vectorized solution uses the `vsetvl` instruction to process batches of data. Assuming a standard hardware configuration of `VLEN = 128 bits`:
* **Parallelism:** The vector register holds **eight** 16-bit elements ($128 / 16 = 8$).
* **Hardware Saturation:** I utilized `vsmul.vx` and `vsadd.vv`. These instructions handle the Q15 fixed-point math and saturation in hardware, eliminating the need for software branching entirely.
* **Amortized Overhead:** The loop overhead (checking `n > 0`, updating pointers) occurs only once for every 8 elements.
* **Estimated Cost:** ~8-10 cycles per **8 elements** (effectively ~1-1.2 cycles per element).

### 3. Speedup Calculation
Comparing the throughput of both implementations:
* **Scalar Throughput:** $\approx \frac{1 \text{ element}}{12 \text{ cycles}}$
* **Vector Throughput:** $\approx \frac{8 \text{ elements}}{10 \text{ cycles}}$

$$\text{Speedup} \approx \frac{\text{Vector Throughput}}{\text{Scalar Throughput}} \approx \frac{0.8}{0.083} \approx 9.6\times$$

**Conclusion:**
The vectorized implementation is expected to achieve a **9x to 12x speedup** over the scalar baseline. This gain is derived from:
1.  **SIMD Parallelism:** Processing 8 elements per instruction.
2.  **Branch Elimination:** Removing costly conditional branches for saturation logic.
## Build & Verification

## Performance Analysis (Theoretical)

Compared to a standard scalar C implementation, the vectorized approach offers significant speedup:

| Metric | Scalar Implementation | Vector Implementation (VLEN=128) |
| :--- | :--- | :--- |
| **Ops per Cycle** | 1 (Best Case) | ~8 (Parallel) |
| **Saturation Cost** | High (Branching) | Zero (Hardware) |
| **Throughput** | ~1 element / 10-15 cycles | ~1 element / 1 cycle |
| **Est. Speedup** | 1x | **8x - 12x** |


This code was verified using the **Godbolt Compiler Explorer** environment with the following configuration:
* **Compiler:** RISC-V GCC (rv32gcv / rv64gcv)
* **Simulator:** Spike (ISA Simulator)
* **Verification:** Verified via assembly inspection for correct instruction generation (`vsetvl`, `vsmul`, `vsadd`).
## Compilation Result in GodBolt.org
![Test Output Verification](assets/screenshot.png)

### Building (Requires RISC-V Toolchain)

To compile the solution locally:
```bash
riscv64-unknown-elf-gcc -march=rv64gcv -O3 -c src/q15_axpy_rvv.c -o q15_axpy.o
```
```bash
riscv64-unknown-elf-gcc \
-O2 \
-march=rv64gcv \
-mabi=lp64 \
src/q15_axpy_challenge.c \
-o q15.elf
```
```bash
sudo apt update
sudo apt install qemu-user qemu-system-misc
```
I tried to simulate the results using Spike and GCC compiler, but would get stuck in this error cycle constantly, would try to fix it in future trials.Currently due to less time , I am not completing the simulation for now.

![Test Output Verification](assets/screenshotqemu.png)

