# CPU Pipeline

This document provides a **detailed walkthrough of the 5-stage pipeline** of the RISC-V CPU. The pipeline stages are Instruction Fetch (IF), Instruction Decode (ID), Execute (EX), Memory Access (MEM), and Write-Back (WB). Each stage is connected via pipeline registers, and control signals manage the flow of instructions and data.

The CPU is designed to work with **ideal memory units**, meaning all memory accesses complete in a single cycle.

---

## Pipeline Stages

### 1. Instruction Fetch (IF)
- **Purpose:** Fetches the instruction from memory based on the Program Counter (PC).  
- **Operations:**
  - Read instruction from instruction memory
  - Increment or update the PC for the next instruction
- **Notes:** Forwarding and branch logic determine the next PC to avoid control hazards.  

### 2. Instruction Decode (ID)
- **Purpose:** Decodes the fetched instruction and prepares operands.  
- **Operations:**
  - Decode opcode and function fields
  - Read source registers from the register file
  - Generate control signals for ALU, memory, and write-back stages

### 3. Execute (EX)
- **Purpose:** Performs arithmetic, logic or address calculations.  
- **Operations:**
  - ALU computes arithmetic or logical operations
  - Branch decisions are evaluated
  - Compute memory addresses for load/store instructions
- **Notes:** Data forwarding is applied to reduce stalls for dependent instructions.  

### 4. Memory Access (MEM)
- **Purpose:** Access data memory for load and store instructions.  
- **Operations:**
  - Read from or write to memory
  - Pass results to write-back stage
- **Notes:** Since memory is assumed ideal, accesses complete in **0 cycles**.  

### 5. Write-Back (WB)
- **Purpose:** Write results back to the register file.  
- **Operations:**
  - Write ALU results or loaded memory values to destination registers
- **Notes:** Updates the register file for subsequent instructions.  

---

## Pipeline Diagram Reference
![CPU Logic Diagram](../../assets/cpu_logic_diagram.png)  
*Figure 1: Logic diagram of the 5-stage pipelined RISC-V CPU.*

---

## Summary

This pipeline allows **instruction-level parallelism**, improving throughput by executing multiple instructions simultaneously across different stages. Forwarding and branch logic help mitigate data and control hazards, ensuring correct execution without unnecessary stalls.
