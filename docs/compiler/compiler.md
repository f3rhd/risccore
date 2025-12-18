# FS Compiler
![FS Compiler Pipeline](../assets/compiler_pipeline.svg)
*Figure 1: Block diagram of the FS compiler pipeline, showing how FS source code is transformed into machine code.*

The FS compiler translates a simple C-like high-level language (FS) into **RISC-V assembly** compatible with the 5-stage pipelined CPU in this repository. It also provides an **IR and AST view** for debugging and learning purposes.

## 1. Compiler Pipeline Overview

The FS compiler translates high-level FS code into RISC-V assembly in multiple stages:

1. **Lexical Analysis (Lexer)** – Converts source code into tokens.
2. **Parsing (Parser)** – Builds an Abstract Syntax Tree (AST) from tokens.
3. **Semantic Analysis** – Checks types, variable declarations, and operation validity.
4. **Intermediate Representation (IR) Generation** – Converts the AST into a simpler, linear IR.
5. **Register Allocation** – Allocates registers for variables using techniques such as register coloring.
5. **Assembly Generation** – Translates IR into RISC-V assembly for the 5-stage CPU.
6. **Assembling** – Converts the generated assembly code to machine code.

## 2. How to build
The FS compiler is purely written in C++ and consists of multiple `.cpp` files in the `compiler/` directory, plus the assembler source code.  
To avoid compiling the assembler `main.cpp` into the compiler, you need to define a macro `-DASSEMBLER_STATIC_BUILD`.
```bash
    g++ compiler/assembler/code_gen/*.cpp compiler/assembler/common/*.cpp compiler/assembler/parser/*.cpp compiler/assembler/preprocessor/*.cpp compiler/assembler/tokenizer/*.cpp compiler/assembler/main.cpp compiler/compiler/common/ast/nodes.cpp compiler/compiler/lexer/lexer.cpp compiler/compiler/parser/fs_parser.cpp compiler/compiler/program/program.cpp compiler/compiler/main.cpp -std=c++14  -w -O2 -DASSEMBLER_STATIC_BUILD -o fsrcv
```
## 3. Compilation commands
```bash
    ./fsrvc <input_file> --emit-asm <output_file> # Generates asm file
    ./fsrvc <input_file> --emit-asm cout # Displays the asm on the terminal
    ./fsrvc <input_file> --emit-asm-debug <output_file> # Displays the detailed information about generated assembly instructions on the terminal
    ./fsrvc <input_file> --emit-ir-debug <output_file> # Prints the detailed information of intermediate instructions in json format to the output file
    ./fsrvc <input_file> --emit-ir <output_file> # Generates ir file that holds intermediate representation
    ./fsrvc <input_file> --emit-ir cout # Displays the ir on the terminal
    ./fsrvc <input_file> --emit-ir-debug cout # Displays the detailed information of irs on the terminal
    ./fsrvc <input_file> --print-ast # Displays the AST on the terminal
```
