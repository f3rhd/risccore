.reset_vector:
	li sp, 0xFFFF0
	call .main
.bubble_sort:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	sw      a0,-12(s0)
	sw      a1,-16(s0)
	li      t0,0
	sw      t0,-20(s0)
	j       .L0
.L0:
	li      t3,0
	sw      t3,-24(s0)
	li      t0,0
	sw      t0,-28(s0)
	j       .L2
.L2:
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t1,0(t0)
	lw      t3,-24(s0)
	addi    t0,t3,1
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	ble     t1,t0,.L4
.L6:
	lw      t3,-24(s0)
	addi    t0,t3,1
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-32(s0)
	lw      t3,-24(s0)
	addi    t0,t3,1
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t4,t2,t0
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	sw      t0,0(t4)
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t1,-32(s0)
	sw      t1,0(t0)
	li      t0,1
	sw      t0,-28(s0)
.L4:
	nop     
.L3:
	lw      t3,-24(s0)
	addi    t3,t3,1
	sw      t3,-24(s0)
	lw      t1,-16(s0)
	lw      t0,-20(s0)
	sub     t0,t1,t0
	li      t6,1
	sub     t0,t0,t6
	lw      t3,-24(s0)
	blt     t3,t0,.L2
.L7:
	nop     
.L8:
	lw      t0,-28(s0)
	bne     t0,zero,.L9
.L11:
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.L9:
	nop     
.L1:
	lw      t0,-20(s0)
	addi    t0,t0,1
	sw      t0,-20(s0)
	lw      t0,-20(s0)
	lw      t1,-16(s0)
	blt     t0,t1,.L0
.L12:
	nop     
.L13:
	nop     
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.main:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	li      t0,5
	sw      t0,-32(s0)
	li      t0,4
	sw      t0,-28(s0)
	li      t0,3
	sw      t0,-24(s0)
	li      t0,2
	sw      t0,-20(s0)
	li      t0,1
	sw      t0,-16(s0)
	addi    t6,s0,-32
	mv      a0,t6
	li      a1,5
	call    .bubble_sort
	mv      t0,a0
	li      t6,0
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,1
	bne     t0,t6,.L14
.L16:
	li      t6,1
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,2
	bne     t0,t6,.L14
.L17:
	li      t6,2
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,3
	bne     t0,t6,.L14
.L18:
	li      t6,3
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,4
	bne     t0,t6,.L14
.L19:
	li      t6,4
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,5
	bne     t0,t6,.L14
.L20:
	j       .L21
.L22:
	nop     
.L21:
	li      t6,1
	bne     t6,zero,.L22
.L23:
	nop     
.L24:
	nop     
.L14:
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
