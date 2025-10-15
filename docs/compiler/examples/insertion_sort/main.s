.reset_vector:
	li sp, 0xFFFF0
	call .main
.insertion_sort:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	sw      a0,-12(s0)
	sw      a1,-16(s0)
	li      t4,1
	sw      t4,-20(s0)
	li      t3,0
	sw      t3,-24(s0)
	li      t1,0
	sw      t1,-28(s0)
	j       .L0
.L0:
	lw      t4,-20(s0)
	slli    t0,t4,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	mv      t1,t0
	sw      t1,-28(s0)
	lw      t4,-20(s0)
	li      t6,1
	sub     t0,t4,t6
	mv      t3,t0
	sw      t3,-24(s0)
	j       .L2
.L3:
	lw      t3,-24(s0)
	addi    t0,t3,1
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t1,t2,t0
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	sw      t0,0(t1)
	lw      t3,-24(s0)
	li      t6,1
	sub     t0,t3,t6
	mv      t3,t0
	sw      t3,-24(s0)
.L2:
	lw      t3,-24(s0)
	blt     t3,zero,.L4
.L5:
	lw      t3,-24(s0)
	slli    t0,t3,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t0,0(t0)
	lw      t1,-28(s0)
	bgt     t0,t1,.L3
.L6:
	nop     
.L4:
	nop     
.L7:
	lw      t3,-24(s0)
	addi    t0,t3,1
	slli    t0,t0,2
	lw      t2,-12(s0)
	add     t0,t2,t0
	lw      t1,-28(s0)
	sw      t1,0(t0)
.L1:
	lw      t4,-20(s0)
	addi    t4,t4,1
	sw      t4,-20(s0)
	lw      t4,-20(s0)
	lw      t0,-16(s0)
	blt     t4,t0,.L0
.L8:
	nop     
.L9:
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
	sw      t0,-8(s0)
	lw      t6,-8(s0)
	li      t0,4
	sw      t0,-4(s0)
	lw      t6,-8(s0)
	li      t0,3
	sw      t0,0(s0)
	lw      t6,-8(s0)
	li      t0,2
	sw      t0,4(s0)
	lw      t6,-8(s0)
	li      t0,1
	sw      t0,8(s0)
	addi    t6,s0,-32
	mv      a0,t6
	li      a1,5
	lw      t6,-8(s0)
	lw      t6,-8(s0)
	call    .insertion_sort
	mv      t0,a0
	li      t6,0
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t6,-8(s0)
	lw      t0,0(t0)
	li      t6,1
	bne     t0,t6,.L10
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L12:
	li      t6,1
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t6,-8(s0)
	lw      t0,0(t0)
	li      t6,2
	bne     t0,t6,.L10
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L13:
	li      t6,2
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t6,-8(s0)
	lw      t0,0(t0)
	li      t6,3
	bne     t0,t6,.L10
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L14:
	li      t6,3
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t6,-8(s0)
	lw      t0,0(t0)
	li      t6,4
	bne     t0,t6,.L10
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L15:
	li      t6,4
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t6,-8(s0)
	lw      t0,0(t0)
	li      t6,5
	bne     t0,t6,.L10
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L16:
	lw      t6,-8(s0)
	lw      t6,-8(s0)
	j       .L17
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L18:
	lw      t6,-8(s0)
	lw      t6,-8(s0)
	nop     
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L17:
	li      t6,1
	bne     t6,zero,.L18
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L19:
	lw      t6,-8(s0)
	lw      t6,-8(s0)
	nop     
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L20:
	lw      t6,-8(s0)
	lw      t6,-8(s0)
	nop     
	lw      t6,-8(s0)
	lw      t6,-8(s0)
.L10:
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
