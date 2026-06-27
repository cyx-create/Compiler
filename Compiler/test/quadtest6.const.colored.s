.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #24
	mov r0, r0
	bl malloc
	mov r5, r0
	movw r0, #5
	str r0, [r5]
	movw r0, #1
	str r0, [r5, #4]
	movw r0, #2
	str r0, [r5, #8]
	movw r0, #3
	str r0, [r5, #12]
	movw r0, #4
	str r0, [r5, #16]
	movw r0, #5
	str r0, [r5, #20]
	mov r0, #0
	mov r0, r0
main$L102:
main$L100108:
	ldr r1, [r5]
	cmp r0, r1
	blt main$L103
main$L104:
	mov r4, r0
	movw r1, #1
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r4, r0
	ldr r6, [r5]
	movw r0, #0
	cmp r4, r0
	bge main$L106
main$L105:
	mov r0, r4
main$L100109:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L106:
	mov r0, r4
	cmp r0, r6
	bge main$L105
main$L107:
	mov r4, r0
	add r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r5, r0]
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	add r0, r4, #1
	mov r0, r0
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
