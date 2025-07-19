# Load instructions
lui     t0, 0x12345          # t0 = 0x12345000
auipc   t1, 0x1              # t1 = PC + 0x1000

addi    sp, zero, 0x100      # initialize stack
addi    t2, zero, 8          # t2 = 8
addi    t3, zero, 0x10       # t3 = 16

# Store data into memory
sw      t2, 0(sp)            # store t2 at sp
sh      t3, 4(sp)            # store t3 at sp+4
sb      t2, 6(sp)            # store byte from t2 at sp+6

# Load data back
lw      t4, 0(sp)            # t4 = 8
lh      t5, 4(sp)            # t5 = 16
lb      t6, 6(sp)            # t6 = 8 (signed)
lbu     a0, 6(sp)            # a0 = 8 (unsigned)
lhu     a1, 4(sp)            # a1 = 16 (unsigned)

# Immediate ALU
addi    a2, t4, 1            # a2 = 9
slti    a3, t4, 10           # a3 = 1
sltiu   a4, t4, 10           # a4 = 1
xori    a5, t4, 0xFF         # a5 = t4 ^ 0xFF
ori     s0, t4, 0xF0         # s0 = t4 | 0xF0
andi    s1, t4, 0xF0         # s1 = t4 & 0xF0
slli    s2, t4, 1            # s2 = t4 << 1
srli    s3, t4, 1            # s3 = t4 >> 1 (logical)
srai    s4, t4, 1            # s4 = t4 >> 1 (arithmetic)

# Register ALU
add     s5, t4, t5           # s5 = t4 + t5
sub     s6, t4, t5           # s6 = t4 - t5
sll     s7, t4, t5           # s7 = t4 << t5[4:0]
slt     s8, t4, t5           # s8 = (t4 < t5)
sltu    s9, t4, t5           # s9 = (unsigned t4 < t5)
xor     s10, t4, t5          # s10 = t4 ^ t5
srl     s11, t4, t5          # s11 = t4 >> t5 (logical)
sra     a6, t4, t5           # a6 = t4 >> t5 (arithmetic)
or      a7, t4, t5           # a7 = t4 | t5
and     t0, t4, t5           # t0 = t4 & t5

# Branch testing
addi    t1, zero, 5
addi    t2, zero, 5
addi    t3, zero, 7

beq     t1, t2, label_beq    # should jump
nop
label_beq:
bne     t1, t3, label_bne    # should jump
nop
label_bne:
blt     t1, t3, label_blt    # should jump
nop
label_blt:
bge     t3, t1, label_bge    # should jump
nop
label_bge:
bltu    t1, t3, label_bltu   # should jump (unsigned)
nop
label_bltu:
bgeu    t3, t1, label_bgeu   # should jump (unsigned)
nop
label_bgeu:

# Jump and link
jal     t0, label_jal        # PC -> t0
nop
label_jal:
jalr    ra, t0, 0            # jump back to t0 (simulate ret)

