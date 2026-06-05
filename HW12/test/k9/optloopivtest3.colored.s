.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r5, #0
	bl getint
	mov r4, r0
	bl getint
	mov r10, r0
	str r10, [fp, #-40]
main$L102:
	movw r0, #0
	cmp r4, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r5
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	sub r4, r4, r9
	movw r0, #8
	mul r0, r0, r4
	add r5, r0, #7
	mov r0, r5
	bl putint
	movw r0, #32
	bl putch
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
