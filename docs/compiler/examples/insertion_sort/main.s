.reset_vector:
	li sp, 0xFFFF0
	call .main
.min:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	sw      a0,-12(s0)
	sw      a1,-16(s0)
	li      t6,0
	slli    t0,t6,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-20(s0)
	li      t0,1
	sw      t0,-24(s0)
	j       .L0
.L0:
	lw      t0,-24(s0)
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	lw      t1,-20(s0)
	bge     t0,t1,.L2
	lw      t0,-24(s0)
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-20(s0)
.L2:
	nop     
.L1:
	lw      t0,-24(s0)
	addi    t0,t0,1
	sw      t0,-24(s0)
	lw      t0,-24(s0)
	lw      t1,-16(s0)
	blt     t0,t1,.L0
.L4:
	lw      t1,-20(s0)
	mv      a0,t1
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.max:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	sw      a0,-12(s0)
	sw      a1,-16(s0)
	li      t6,0
	slli    t0,t6,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-20(s0)
	li      t3,1
	sw      t3,-24(s0)
	j       .L5
.L5:
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	lw      t1,-20(s0)
	ble     t0,t1,.L7
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-20(s0)
.L7:
	nop     
.L6:
	lw      t3,-24(s0)
	addi    t3,t3,1
	sw      t3,-24(s0)
	lw      t3,-24(s0)
	lw      t0,-16(s0)
	blt     t3,t0,.L5
.L9:
	lw      t1,-20(s0)
	mv      a0,t1
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.main:
	addi    sp,sp,-48
	sw      ra,44(sp)
	sw      s0,40(sp)
	addi    s0,sp,48
	li      t0,1
	sw      t0,-28(s0)
	li      t0,2
	sw      t0,-24(s0)
	li      t0,3
	sw      t0,-20(s0)
	li      t0,4
	sw      t0,-16(s0)
	li      t0,5
	sw      t0,-12(s0)
	addi    t6,s0,-28
	mv      a0,t6
	li      a1,5
	call    .max
	mv      t0,a0
	sw      t0,-32(s0)
	addi    t6,s0,-28
	mv      a0,t6
	li      a1,5
	call    .min
	mv      t0,a0
	sw      t0,-36(s0)
	lw      t0,-36(s0)
	li      t6,1
	bne     t0,t6,.L10
	lw      t0,-32(s0)
	li      t6,5
	bne     t0,t6,.L10
	j       .L12
.L13:
	nop     
.L12:
	li      t6,1
	bne     t6,zero,.L13
.L14:
	nop     
.L10:
	lw      ra,44(sp)
	lw      s0,40(sp)
	addi    sp,sp,48
	jr      ra
