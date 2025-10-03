# CPU Instruction Set

> ⚠️ **Note:** Instruction formats and pseudoinstructions are based on *Digital Design and Computer Architecture: RISC-V Edition* by Sarah L. Harris and David Money Harris.

This document lists all **instructions supported by the 5-stage pipelined RISC-V CPU**. Instructions are grouped by type, with their **opcode, funct3/funct7 (if applicable), and operation**.

---

## 1. R-Type Instructions (Register-Register)

| Opcode       | funct3 | funct7   | Mnemonic | Description                     | Operation                        |
|---------     |--------|----------|----------|---------------------------------|----------------------------------|
| 0110011 (51) | 000    | 0000000  | add      | Add two registers               | rd = rs1 + rs2                   |
| 0110011 (51) | 000    | 0100000  | sub      | Subtract                        | rd = rs1 - rs2                   |
| 0110011 (51) | 001    | 0000000  | sll      | Shift left logical              | rd = rs1 << rs2[4:0]             |
| 0110011 (51) | 010    | 0000000  | slt      | Set less than                   | rd = (rs1 < rs2) ? 1 : 0         |
| 0110011 (51) | 011    | 0000000  | sltu     | Set less than unsigned          | rd = (rs1 < rs2 unsigned) ? 1 : 0|
| 0110011 (51) | 100    | 0000000  | xor      | Bitwise XOR                     | rd = rs1 ^ rs2                   |
| 0110011 (51) | 101    | 0000000  | srl      | Shift right logical             | rd = rs1 >> rs2[4:0]             |
| 0110011 (51) | 101    | 0100000  | sra      | Shift right arithmetic          | rd = rs1 >>> rs2[4:0]            |
| 0110011 (51) | 110    | 0000000  | or       | Bitwise OR                      | rd = rs1 \| rs2                  |
| 0110011 (51) | 111    | 0000000  | and      | Bitwise AND                     | rd = rs1 & rs2                   |
| 0110011 (51) | 000    | 0000001  | mul      | Multiply                        | rd = (rs1 * rs2)[31:0]           |
| 0110011 (51) | 001    | 0000001  | mulh     | Multiply high signed-signed     | rd = (rs1 * rs2)[63:32]          |
| 0110011 (51) | 010    | 0000001  | mulhsu   | Multiply high signed-unsigned   | rd = (rs1 * rs2)[63:32]          |
| 0110011 (51) | 011    | 0000001  | mulhu    | Multiply high unsigned-unsigned | rd = (rs1 * rs2)[63:32]          |
| 0110011 (51) | 100    | 0000001  | div      | Divide signed                   | rd = rs1 / rs2                   |
| 0110011 (51) | 101    | 0000001  | divu     | Divide unsigned                 | rd = rs1 / rs2                   |
| 0110011 (51) | 110    | 0000001  | rem      | Remainder signed                | rd = rs1 % rs2                   |
| 0110011 (51) | 111    | 0000001  | remu     | Remainder unsigned              | rd = rs1 % rs2                   |

---

## 2. I-Type Instructions (Immediate / Loads / jalr)

| Opcode       | funct3 | Mnemonic | Description                      | Operation                                  |
|--------------|--------|----------|----------------------------------|--------------------------------------------|
| 0010011 (19) | 000    | addi     | Add immediate                    | rd = rs1 + SignExt(imm)                    |
| 0010011 (19) | 010    | slti     | Set less than immediate          | rd = (rs1 < SignExt(imm)) ? 1 : 0          |
| 0010011 (19) | 011    | sltiu    | Set less than immediate unsigned | rd = (rs1 < SignExt(imm) unsigned) ? 1 : 0 |
| 0010011 (19) | 100    | xori     | XOR immediate                    | rd = rs1 ^ SignExt(imm)                    |
| 0010011 (19) | 110    | ori      | OR immediate                     | rd = rs1 \| SignExt(imm)                   |
| 0010011 (19) | 111    | andi     | AND immediate                    | rd = rs1 & SignExt(imm)                    |
| 0010011 (19) | 001    | slli     | Shift left logical immediate     | rd = rs1 << uimm                           |
| 0010011 (19) | 101    | srli     | Shift right logical immediate    | rd = rs1 >> uimm                           |
| 0010011 (19) | 101    | srai     | Shift right arithmetic immediate | rd = rs1 >>> uimm                          |
| 0000011 (3)  | 000    | lb       | Load byte                        | rd = SignExt([Address]7:0)                 |
| 0000011 (3)  | 001    | lh       | Load half                        | rd = SignExt([Address]15:0)                |
| 0000011 (3)  | 010    | lw       | Load word                        | rd = [Address]31:0                         |
| 0000011 (3)  | 100    | lbu      | Load byte unsigned               | rd = ZeroExt([Address]7:0)                 |
| 0000011 (3)  | 101    | lhu      | Load half unsigned               | rd = ZeroExt([Address]15:0)                |
| 1100111 (103)| 000    | jalr     | Jump and link register           | PC = rs1 + SignExt(imm), rd = PC + 4       |

---

## 3. S-Type Instructions (Stores)

| Opcode      | funct3 | Mnemonic | Description | Operation                 |
|-------------|--------|----------|-------------|---------------------------|
| 0100011 (35)| 000    | sb       | Store byte  | [Address]7:0 = rs2[7:0]   |
| 0100011 (35)| 001    | sh       | Store half  | [Address]15:0 = rs2[15:0] |
| 0100011 (35)| 010    | sw       | Store word  | [Address]31:0 = rs2       |

---

## 4. B-Type Instructions (Branches)

| Opcode      | funct3 | Mnemonic | Description                      | Operation                        |
|-------------|--------|----------|----------------------------------|----------------------------------|
| 1100011 (99)| 000    | beq      | Branch if equal                  | if (rs1 == rs2) PC = BTA         |
| 1100011 (99)| 001    | bne      | Branch if not equal              | if (rs1 ≠ rs2) PC = BTA          |
| 1100011 (99)| 100    | blt      | Branch if less than              | if (rs1 < rs2) PC = BTA          |
| 1100011 (99)| 101    | bge      | Branch if greater/equal          | if (rs1 ≥ rs2) PC = BTA          |
| 1100011 (99)| 110    | bltu     | Branch if less than unsigned     | if (rs1 < rs2 unsigned) PC = BTA |
| 1100011 (99)| 111    | bgeu     | Branch if greater/equal unsigned | if (rs1 ≥ rs2 unsigned) PC = BTA |

---

## 5. U-Type Instructions

| Opcode      | Mnemonic | Description                   | Operation                |
|-------------|----------|-------------------------------|--------------------------|
| 0110111 (55)| lui      | Load upper immediate          | rd = {upimm, 12'b0}      |
| 0010111 (23)| auipc    | Add upper immediate to PC     | rd = {upimm, 12'b0} + PC |

---

## 6. J-Type Instructions

| Opcode       | Mnemonic | Description                   | Operation             |
|------------- |----------|-------------------------------|-----------------------|
| 1101111 (111)| jal      | Jump and link                 | PC = JTA, rd = PC + 4 |
| —            | j        | Pseudoinstruction for jal     | PC = JTA              |
| —            | jr       | Pseudoinstruction for jalr    | PC = rs1              |

---

## 7. Pseudoinstructions


| Pseudoinstr          | Expansion                             | Description                                        |
|----------------------|---------------------------------------|----------------------------------------------------|
| nop                  | addi x0, x0, 0                        | No operation                                       |
| li rd, imm           | lui/addi sequence                     | Load 32-bit immediate into rd                      |
| mv rd, rs1           | addi rd, rs1, 0                       | Register copy (move value from rs1 to rd)          |
| not rd, rs1          | xori rd, rs1, -1                      | One’s complement (~rs1)                            |
| neg rd, rs1          | sub rd, x0, rs1                       | Two’s complement (-rs1)                            |
| seqz rd, rs1         | sltiu rd, rs1, 1                      | Set rd = 1 if rs1 == 0, else rd = 0                |
| snez rd, rs1         | sltu rd, x0, rs1                      | Set rd = 1 if rs1 != 0, else rd = 0                |
| sltz rd, rs1         | slt rd, rs1, x0                       | Set rd = 1 if rs1 < 0, else rd = 0                 |
| sgtz rd, rs1         | slt rd, x0, rs1                       | Set rd = 1 if rs1 > 0, else rd = 0                 |
| beqz rs1, label      | beq rs1, x0, label                    | Branch to label if rs1 == 0                        |
| bnez rs1, label      | bne rs1, x0, label                    | Branch to label if rs1 != 0                        |
| blez rs1, label      | bge x0, rs1, label                    | Branch to label if rs1 ≤ 0                         |
| bgez rs1, label      | bge rs1, x0, label                    | Branch to label if rs1 ≥ 0                         |
| bltz rs1, label      | blt rs1, x0, label                    | Branch to label if rs1 < 0                         |
| bgtz rs1, label      | blt x0, rs1, label                    | Branch to label if rs1 > 0                         |
| ble rs1, rs2, label  | bge rs2, rs1, label                   | Branch to label if rs1 ≤ rs2                       |
| bgt rs1, rs2, label  | blt rs2, rs1, label                   | Branch to label if rs1 > rs2                       |
| bleu rs1, rs2, label | bgeu rs2, rs1, label                  | Branch to label if rs1 ≤ rs2 unsigned              |
| bgtu rs1, rs2, label | bltu rs2, rs1, label                  | Branch to label if rs1 > rs2 unsigned              |
| j label              | jal x0, label                         | Jump to label                                      |
| jal label            | jal ra, label                         | Jump and link (call)                               |
| jr rs1               | jalr x0, rs1, 0                       | Jump register (PC = rs1)                           |
| ret                  | jalr x0, ra, 0                        | Return from function (PC = ra)                     |
| call label           | jal ra, label                         | Call nearby function (PC = label, ra = PC+4)       |
| call label           | auipc ra, offset; jalr ra, ra, offset | Call far away function (PC = PC+offset, ra = PC+4) |
