.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, #1
	sub r4, r0, #1
	mov r0, r1
main$L102:
main$L100105:
	movw r1, #0
	cmp r0, r1
	bne main$L103
main$L104:
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	mov r0, r4
	b main$L100105

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
