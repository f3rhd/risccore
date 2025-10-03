# Pipeline Hazards

In a pipelined CPU, hazards occur when instructions cannot proceed smoothly. This section explains the types of hazards in the **5-stage pipelined RISC-V CPU** and how they are handled in this design.

---

## 1. Data Hazards

Data hazards occur when instructions depend on the result of a previous instruction that has not yet completed.

**Types:**

1. **Read After Write (RAW) – True Dependency**  
   - Happens when an instruction needs a value being computed by a previous instruction.  
   - **Example:**
     ```assembly
     lw x5, 0(x6)    # Instruction 1
     add x7, x5, x8  # Instruction 2 uses x5
     ```
   - **Solution in this CPU:**  
     - Forwarding from **MEM stage to EX stage** allows the `add` instruction to use the loaded value immediately.  
     - No stall occurs for load-use hazards.

2. **Write After Read (WAR) – Anti Dependency**  
   - Not an issue in this in-order pipeline.

3. **Write After Write (WAW) – Output Dependency**  
   - Not an issue in this in-order pipeline.

---

## 2. Control Hazards (Branch Hazards)

Control hazards occur when the CPU does not know the next instruction because a branch outcome is not yet determined.

**Example:**
```assembly
beq x5, x6, label   # Branch instruction
add x7, x8, x9      # Next instruction
```
- **Solution in this CPU:** 
    - Branch target address is forwarded to the input port of the instruction memory.
    - **DEC** stage is flushed.
    - Penalty becomes 1 cycle.
