.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r1
	str r10, [fp, #-44]
	mov r10, r2
	str r10, [fp, #-48]
	mov r0, #0
	movw r1, #1
	ldr r12, [fp, #-48]
	cmp r12, r1
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r10, r0
	str r10, [fp, #-52]
b1$bubbleSort$L107:
	ldr r12, [fp, #-48]
	sub r0, r12, #1
	ldr r12, [fp, #-52]
	cmp r12, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r12, [fp, #-40]
	ldr r3, [r12, #4]
	ldr r12, [fp, #-40]
	mov r0, r12
	ldr r12, [fp, #-44]
	mov r1, r12
	ldr r12, [fp, #-48]
	sub r2, r12, #1
	blx r3
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	ldr r12, [fp, #-44]
	mov r4, r12
	ldr r12, [fp, #-44]
	ldr r5, [r12]
	movw r0, #0
	ldr r12, [fp, #-52]
	cmp r12, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L111:
	ldr r12, [fp, #-52]
	cmp r12, r5
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	ldr r12, [fp, #-52]
	add r0, r12, #1
	ldr r12, [fp, #-52]
	add r6, r12, #1
	ldr r12, [fp, #-44]
	mov r7, r12
	movw r1, #4
	mul r0, r0, r1
	ldr r4, [r4, r0]
	ldr r12, [fp, #-44]
	ldr r5, [r12]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L114:
	cmp r6, r5
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r7, r0]
	cmp r4, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	ldr r12, [fp, #-52]
	add r0, r12, #1
	mov r10, r0
	str r10, [fp, #-52]
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	ldr r12, [fp, #-44]
	mov r4, r12
	ldr r12, [fp, #-44]
	ldr r5, [r12]
	movw r0, #0
	ldr r12, [fp, #-52]
	cmp r12, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L119:
	ldr r12, [fp, #-52]
	cmp r12, r5
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	ldr r12, [fp, #-40]
	mov r0, r12
	ldr r12, [fp, #-52]
	add r1, r12, #1
	ldr r12, [fp, #-44]
	mov r7, r12
	movw r2, #4
	mul r1, r1, r2
	ldr r1, [r4, r1]
	str r1, [r0]
	ldr r12, [fp, #-44]
	ldr r4, [r12]
	movw r0, #0
	ldr r12, [fp, #-52]
	cmp r12, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L122:
	ldr r12, [fp, #-52]
	cmp r12, r4
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	ldr r12, [fp, #-52]
	add r6, r12, #1
	ldr r12, [fp, #-44]
	mov r4, r12
	ldr r12, [fp, #-44]
	ldr r5, [r12]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L125:
	cmp r6, r5
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	ldr r12, [fp, #-52]
	add r0, r12, #1
	add r1, r6, #1
	ldr r12, [fp, #-52]
	add r5, r12, #1
	ldr r12, [fp, #-44]
	mov r6, r12
	movw r2, #4
	mul r0, r0, r2
	movw r2, #4
	mul r1, r1, r2
	add r0, r7, r0
	ldr r1, [r4, r1]
	str r1, [r0]
	ldr r12, [fp, #-44]
	ldr r4, [r12]
	movw r0, #0
	cmp r5, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r0, #65535
	movt r0, #65535
	bl exit
b1$bubbleSort$L128:
	cmp r5, r4
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r1, r5, #1
	ldr r12, [fp, #-40]
	ldr r0, [r12]
	movw r2, #4
	mul r1, r1, r2
	str r0, [r6, r1]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #32
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	mov r0, #0
	mov r4, #0
	movw r0, #7
	ldr r12, [fp, #-40]
	str r0, [r12]
	movw r0, #6
	ldr r12, [fp, #-40]
	str r0, [r12, #4]
	movw r0, #3
	ldr r12, [fp, #-40]
	str r0, [r12, #8]
	movw r0, #0
	ldr r12, [fp, #-40]
	str r0, [r12, #12]
	movw r0, #5
	ldr r12, [fp, #-40]
	str r0, [r12, #16]
	movw r0, #9
	ldr r12, [fp, #-40]
	str r0, [r12, #20]
	movw r0, #1
	ldr r12, [fp, #-40]
	str r0, [r12, #24]
	movw r0, #2
	ldr r12, [fp, #-40]
	str r0, [r12, #28]
	movw r0, #8
	bl malloc
	ldr r12, [fp, #-40]
	mov r1, r12
	ldr r2, =b1$bubbleSort
	str r2, [r0, #4]
	ldr r2, [r0, #4]
	ldr r12, [fp, #-40]
	ldr r5, [r12]
	mov r3, r2
	mov r2, r5
	blx r3
	mov r10, r4
	str r10, [fp, #-44]
main$L102:
	ldr r12, [fp, #-44]
	mov r0, r12
	ldr r12, [fp, #-40]
	ldr r1, [r12]
	cmp r0, r1
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r12, [fp, #-40]
	mov r4, r12
	ldr r12, [fp, #-40]
	ldr r5, [r12]
	movw r0, #0
	ldr r12, [fp, #-44]
	cmp r12, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r12, [fp, #-44]
	cmp r12, r5
	bge main$L105
main$L107:
	ldr r12, [fp, #-44]
	add r0, r12, #1
	ldr r12, [fp, #-44]
	add r5, r12, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	mov r10, r5
	str r10, [fp, #-44]
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
