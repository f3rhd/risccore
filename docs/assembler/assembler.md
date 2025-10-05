## Assembler
This assembler is a custom implementation of a RISC-V assembler used for debugging and testing the 5-stage pipelined CPU in this repository.
It translates human-readable RISC-V assembly code into machine code that can be executed by the CPU.
### Overview
- Supports a subset of the RISC-V RV32I instruction set (arithmetic, logic, memory, branch, jump).

- Outputs a binary code file suitable for loading into the CPU’s instruction memory.

- Assembler does not support directives such as .globl, .text, .data, etc. Only raw instructions and labels are accepted.

- Supports macros

### How to build
```bash
g++ compiler/assembler/code_gen/*.cpp compiler/assembler/common/*.cpp compiler/assembler/parser/*.cpp compiler/assembler/preprocessor/*.cpp compiler/assembler/tokenizer/*.cpp compiler/assembler/main.cpp -std=c++14  -w -O2 -o assembler
```
### Usage
``` bash
./assembler <input_file.s> <output_file.bin>
```
