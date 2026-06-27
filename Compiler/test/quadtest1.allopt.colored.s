.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #68
	add fp, sp, #100
	mov r10, r0
	str r10, [fp, #-40]
	mov r7, r1
	mov r10, r2
	str r10, [fp, #-44]
	movw r0, #1
	ldr r12, [fp, #-44]
	cmp r12, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r10, r8
	str r10, [fp, #-48]
	ldr r12, [fp, #-48]
	str r12, [fp, #-52]
	ldr r12, [fp, #-52]
	add r0, r12, #1
	movw r2, #4
	mul r10, r0, r2
	str r10, [fp, #-84]
	ldr r12, [fp, #-52]
	add r10, r12, #1
	str r10, [fp, #-72]
	ldr r12, [fp, #-52]
	mov r0, r12
	ldr r12, [fp, #-72]
	add r2, r12, #1
	movw r3, #4
	mul r10, r2, r3
	str r10, [fp, #-88]
	mov r10, r0
	str r10, [fp, #-56]
	ldr r12, [fp, #-56]
	str r12, [fp, #-60]
	ldr r12, [fp, #-60]
	add r0, r12, #1
	movw r2, #4
	mul r10, r0, r2
	str r10, [fp, #-92]
	ldr r12, [fp, #-60]
	mov r0, r12
	mov r4, r0
	add r0, r4, #1
	movw r2, #4
	mul r10, r0, r2
	str r10, [fp, #-96]
	ldr r12, [fp, #-76]
	add r0, r12, #1
	movw r2, #4
	mul r10, r0, r2
	str r10, [fp, #-100]
	add r10, r4, #1
	str r10, [fp, #-80]
	mov r10, r4
	str r10, [fp, #-64]
	add r10, r4, #1
	str r10, [fp, #-76]
	mov r0, r4
	mov r10, r0
	str r10, [fp, #-68]
	ldr r12, [fp, #-80]
	add r0, r12, #1
	movw r2, #4
	mul r5, r0, r2
	ldr r12, [fp, #-68]
	mov r9, r12
	ldr r12, [fp, #-68]
	mov r9, r12
	ldr r12, [fp, #-68]
	add r10, r12, #1
	str r10, [fp, #-56]
	mov r6, r1
b1$bubbleSort$L107:
b1$bubbleSort$L100133:
	ldr r12, [fp, #-44]
	sub r0, r12, #1
	cmp r6, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	mov r8, r6
	ldr r12, [fp, #-40]
	ldr r0, [r12, #4]
	mov r1, r0
	ldr r12, [fp, #-40]
	mov r0, r12
	mov r3, r7
	ldr r12, [fp, #-44]
	sub r2, r12, #1
	mov r12, r1
	mov r0, r0
	mov r1, r3
	mov r2, r2
	blx r12
	mov r0, r0
	mov r0, r0
	sub sp, fp, #100
	add sp, sp, #68
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	mov r8, r6
	ldr r6, [r7]
	movw r0, #0
	cmp r8, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	mov r10, r8
	str r10, [fp, #-48]
b1$bubbleSort$L100134:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L111:
	ldr r12, [fp, #-48]
	cmp r12, r6
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r12, [fp, #-84]
	ldr r6, [r7, r12]
	ldr r8, [r7]
	movw r0, #0
	ldr r12, [fp, #-72]
	cmp r12, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	ldr r12, [fp, #-52]
	mov r0, r12
b1$bubbleSort$L100135:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L114:
	ldr r12, [fp, #-72]
	cmp r12, r8
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	ldr r12, [fp, #-88]
	ldr r0, [r7, r12]
	cmp r6, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
b1$bubbleSort$L100136:
	ldr r12, [fp, #-56]
	mov r6, r12
	b b1$bubbleSort$L100133
b1$bubbleSort$L130:
	ldr r6, [r7]
	movw r0, #0
	cmp r9, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	mov r10, r9
	str r10, [fp, #-56]
b1$bubbleSort$L100137:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L119:
	ldr r12, [fp, #-56]
	cmp r12, r6
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r12, [fp, #-40]
	mov r0, r12
	ldr r12, [fp, #-92]
	ldr r1, [r7, r12]
	str r1, [r0]
	ldr r6, [r7]
	movw r0, #0
	ldr r12, [fp, #-60]
	cmp r12, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	ldr r12, [fp, #-60]
	mov r10, r12
	str r10, [fp, #-64]
b1$bubbleSort$L100138:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L125:
	ldr r12, [fp, #-64]
	cmp r12, r6
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	mov r6, r7
	ldr r8, [r7]
	movw r0, #0
	ldr r12, [fp, #-76]
	cmp r12, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	mov r0, r4
b1$bubbleSort$L100139:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L122:
	ldr r12, [fp, #-76]
	cmp r12, r8
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r12, [fp, #-100]
	ldr r0, [r6, r12]
	ldr r12, [fp, #-96]
	str r0, [r7, r12]
	ldr r6, [r7]
	movw r0, #0
	ldr r12, [fp, #-80]
	cmp r12, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	mov r0, r4
b1$bubbleSort$L100140:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L128:
	ldr r12, [fp, #-80]
	cmp r12, r6
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	ldr r12, [fp, #-40]
	ldr r0, [r12]
	str r0, [r7, r5]
	ldr r12, [fp, #-68]
	mov r9, r12
	b b1$bubbleSort$L100136
b1$bubbleSort$L102:
	movw r0, #0
	mov r0, r0
	sub sp, fp, #100
	add sp, sp, #68
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #32
	mov r0, r0
	bl malloc
	mov r7, r0
	movw r0, #7
	str r0, [r7]
	movw r0, #6
	str r0, [r7, #4]
	movw r0, #3
	str r0, [r7, #8]
	movw r0, #0
	str r0, [r7, #12]
	movw r0, #5
	str r0, [r7, #16]
	movw r0, #9
	str r0, [r7, #20]
	movw r0, #1
	str r0, [r7, #24]
	movw r0, #2
	str r0, [r7, #28]
	mov r6, #0
	movw r0, #8
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =b1$bubbleSort
	str r1, [r0, #4]
	mov r1, r0
	ldr r0, [r1, #4]
	mov r2, r1
	mov r1, r7
	ldr r3, [r7]
	mov r12, r0
	mov r0, r2
	mov r1, r1
	mov r2, r3
	blx r12
	mov r0, r7
	mov r8, r5
	mov r0, r8
	add r1, r0, #1
	movw r2, #4
	mul r1, r1, r2
	add r9, r0, #1
	mov r5, r6
main$L102:
main$L100108:
	mov r0, r5
	ldr r1, [r7]
	cmp r0, r1
	blt main$L103
main$L104:
	movw r0, #10
	mov r0, r0
	bl putch
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r6, [r7]
	movw r0, #0
	cmp r5, r0
	bge main$L106
main$L105:
	mov r8, r5
main$L100109:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L106:
	cmp r8, r6
	bge main$L105
main$L107:
	ldr r0, [r4]
	mov r0, r0
	bl putint
	movw r0, #32
	mov r0, r0
	bl putch
	mov r5, r9
	b main$L100108

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
