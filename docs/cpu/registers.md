# CPU Registers

This document provides an overview of the **registers in the 5-stage pipelined RISC-V CPU**, including their purpose and usage. The CPU includes the **standard RISC-V registers** as well as some **special-purpose registers** for temporary storage, stack operations, and debugging.

---

## 1. General-Purpose Registers

| Register | ABI Name |           Description               |
|----------|----------|-------------------------------------|
| x0       | zero     | Constant 0                          |
| x1       | ra       | Return address                      |
| x2       | sp       | Stack pointer                       |
| x3       | gp       | Global pointer                      |
| x4       | tp       | Thread pointer                      |
| x5-x7    | t0-t2    | Temporary registers                 |
| x8-x9    | s0/fp,s1 | Saved registers / frame pointer     |
| x10-x17  | a0-a7    | Function arguments / return values  |
| x18-x27  | s2-s11   | Saved registers                     |
| x28-x31  | t3-t6    | Temporary registers                 |

> Note: The standard RISC-V ABI names are used for compatibility and readability.

---

## 2. Special-Purpose Registers

| Register | Name |                 Description                    |
|----------|------|------------------------------------------------|
| ra       | ra   | Return address register (used for subroutines) |

> These registers are used primarily by the **FS compiler** and **assembler** to simplify code generation and debugging.

---

## 3. Stack and Temporary Registers

- **Stack Pointer (`sp`)**: Points to the top of the stack.  
- **Temporary Registers (`t0-t6`)**: Used for intermediate calculations to reduce stalls and forwarding hazards.  
- **Return Address (`ra`)**: Stores the return address for function calls (`jal` / `call`).  

---

## 4. Usage Notes

- The **CPU pipeline relies on these registers** for forwarding and hazard resolution.  
---

This overview ensures a clear understanding of the CPU’s register file and how each register is intended to be used in the pipeline and by the FS compiler.
