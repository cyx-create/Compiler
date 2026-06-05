.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #4
	ldr r9, [fp, #-40]
	str r0, [r9]
	movw r0, #1
	ldr r9, [fp, #-40]
	str r0, [r9, #4]
	movw r0, #2
	ldr r9, [fp, #-40]
	str r0, [r9, #8]
	movw r0, #3
	ldr r9, [fp, #-40]
	str r0, [r9, #12]
	movw r0, #4
	ldr r9, [fp, #-40]
	str r0, [r9, #16]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	movw r1, #10
	mov r0, r1
	bl putch
	movw r1, #2
	mov r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r4, r0, #1
	ldr r9, [fp, #-40]
	ldr r5, [r9]
	movw r0, #0
	cmp r4, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r4, r5
	bge main$L105
main$L107:
	add r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-40]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	mov r0, r4
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
