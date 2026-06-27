.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, r0
	str r10, [fp, #-40]
	mov r4, r1
	mov r10, r2
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-48]
	movw r0, #1
	ldr r12, [fp, #-44]
	cmp r12, r0
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
	ldr r12, [fp, #-48]
	mov r0, r12
b1$bubbleSort$L100133:
b1$bubbleSort$L104:
	mov r5, r0
b1$bubbleSort$L107:
b1$bubbleSort$L100134:
	ldr r12, [fp, #-44]
	sub r0, r12, #1
	cmp r5, r0
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	mov r6, r5
b1$bubbleSort$L100135:
	ldr r12, [fp, #-40]
	ldr r0, [r12, #4]
	mov r2, r0
	ldr r12, [fp, #-40]
	mov r0, r12
	mov r1, r4
	ldr r12, [fp, #-44]
	sub r3, r12, #1
	mov r12, r2
	mov r0, r0
	mov r1, r1
	mov r2, r3
	blx r12
	mov r0, r0
	mov r0, r0
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	mov r6, r5
b1$bubbleSort$L100136:
	ldr r5, [r4]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	mov r0, r6
b1$bubbleSort$L100137:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L111:
	mov r0, r6
b1$bubbleSort$L100138:
	cmp r0, r5
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	mov r8, r0
b1$bubbleSort$L100139:
	add r0, r8, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r5, [r4, r0]
	add r6, r8, #1
	ldr r7, [r4]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	mov r0, r8
b1$bubbleSort$L100140:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L114:
	mov r0, r8
b1$bubbleSort$L100141:
	cmp r6, r7
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	mov r9, r0
b1$bubbleSort$L100142:
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	cmp r5, r0
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
	mov r6, r9
b1$bubbleSort$L100143:
b1$bubbleSort$L132:
b1$bubbleSort$L100144:
	add r0, r6, #1
	mov r5, r0
	b b1$bubbleSort$L100134
b1$bubbleSort$L130:
	mov r6, r9
b1$bubbleSort$L100145:
	ldr r5, [r4]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	mov r0, r6
b1$bubbleSort$L100146:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L119:
	mov r0, r6
b1$bubbleSort$L100147:
	cmp r0, r5
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	mov r6, r0
b1$bubbleSort$L100148:
	ldr r12, [fp, #-40]
	mov r0, r12
	add r1, r6, #1
	movw r2, #4
	mul r1, r1, r2
	ldr r1, [r4, r1]
	str r1, [r0]
	mov r5, r4
	ldr r7, [r4]
	movw r0, #0
	cmp r6, r0
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	mov r0, r6
b1$bubbleSort$L100149:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L125:
	mov r0, r6
b1$bubbleSort$L100150:
	cmp r0, r7
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	mov r8, r0
b1$bubbleSort$L100151:
	add r7, r8, #1
	ldr r6, [r4]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	mov r0, r8
b1$bubbleSort$L100152:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L122:
	mov r0, r8
b1$bubbleSort$L100153:
	cmp r7, r6
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	mov r6, r0
b1$bubbleSort$L100154:
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	add r0, r5, r0
	add r1, r7, #1
	movw r2, #4
	mul r1, r1, r2
	ldr r1, [r4, r1]
	str r1, [r0]
	add r7, r6, #1
	ldr r5, [r4]
	movw r0, #0
	cmp r7, r0
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	mov r0, r6
b1$bubbleSort$L100155:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
b1$bubbleSort$L128:
	mov r0, r6
b1$bubbleSort$L100156:
	cmp r7, r5
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	mov r0, r0
b1$bubbleSort$L100157:
	add r1, r7, #1
	movw r2, #4
	mul r2, r1, r2
	ldr r12, [fp, #-40]
	ldr r1, [r12]
	str r1, [r4, r2]
	mov r6, r0
	b b1$bubbleSort$L100144
b1$bubbleSort$L102:
	ldr r12, [fp, #-48]
	mov r0, r12
b1$bubbleSort$L100158:
	movw r0, #0
	mov r0, r0
	sub sp, fp, #48
	add sp, sp, #16
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
	mov r5, r0
	movw r0, #7
	str r0, [r5]
	movw r0, #6
	str r0, [r5, #4]
	movw r0, #3
	str r0, [r5, #8]
	movw r0, #0
	str r0, [r5, #12]
	movw r0, #5
	str r0, [r5, #16]
	movw r0, #9
	str r0, [r5, #20]
	movw r0, #1
	str r0, [r5, #24]
	movw r0, #2
	str r0, [r5, #28]
	mov r0, #0
	mov r4, #0
	movw r0, #8
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =b1$bubbleSort
	str r1, [r0, #4]
	mov r1, r0
	ldr r0, [r1, #4]
	mov r2, r1
	mov r1, r5
	ldr r3, [r5]
	mov r12, r0
	mov r0, r2
	mov r1, r1
	mov r2, r3
	blx r12
	mov r4, r4
main$L102:
main$L100108:
	mov r0, r4
	ldr r1, [r5]
	cmp r0, r1
	blt main$L103
main$L104:
	mov r6, r4
main$L100109:
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
	mov r6, r4
main$L100110:
	mov r4, r5
	ldr r7, [r5]
	movw r0, #0
	cmp r6, r0
	bge main$L106
main$L105:
	mov r0, r6
main$L100111:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L106:
	mov r0, r6
main$L100112:
	cmp r0, r7
	bge main$L105
main$L107:
	mov r6, r0
main$L100113:
	add r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	mov r0, r0
	bl putint
	movw r0, #32
	mov r0, r0
	bl putch
	add r0, r6, #1
	mov r4, r0
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
