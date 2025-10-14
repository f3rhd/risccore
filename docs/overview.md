![CPU Logic Diagram](assets/cpu_logic_diagram.png)
*Figure 1: Logic diagram of the 5-stage pipelined RISC-V CPU.*
---

## Overview

This repository contains a **5-stage pipelined RISC-V CPU** designed for educational and **custom made C like high level language(FS) along with custom RISCV-assembler** for debugging purposes. The CPU implements the classic five stages: Instruction Fetch (IF), Instruction Decode (ID), Execute (EX), Memory Access (MEM), and Write-Back (WB). Each stage is connected via pipeline registers, and control signals manage data flow and instruction sequencing.

The CPU supports a **subset of the RISC-V instruction set**, including arithmetic, logic, memory access, and control flow instructions. Data hazards are resolved using forwarding mechanisms and control hazards are handled with simple branch logic. 

Registers include the standard RISC-V set, as well as special-purpose registers for temporary storage and stack operations. **Memory units in this system are assumed to be ideal, with 0-cycle access time**, so memory reads and writes complete within a single clock cycle. The CPU works in tandem with a **custom assembler and the FS compiler**, which translates high-level programs into RISC-V assembly for execution on this CPU.

This diagram highlights the **data paths and control signals**, giving a complete view of how instructions flow through the pipeline from fetch to write-back.