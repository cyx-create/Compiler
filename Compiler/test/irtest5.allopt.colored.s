.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, #1
	mov r4, r0
	sub r5, r4, #1
	mov r1, r1
main$L105:
main$L100108:
	movw r0, #0
	cmp r1, r0
	bgt main$L104
main$L107:
	mov r0, r1
main$L100109:
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	mov r0, r1
	movw r1, #65535
	movt r1, #65535
	cmp r0, r1
	bgt main$L106
main$L106:
	mov r0, r4
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	mov r1, r5
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
