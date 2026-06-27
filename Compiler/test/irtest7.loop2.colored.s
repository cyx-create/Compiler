.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r5, #1
	mov r4, #2
	bl getint
	mov r0, r0
	mov r1, #0
	movw r2, #1
	cmp r5, r2
	bgt main$L100108
main$L104:
	mov r2, r1
main$L100109:
	movw r3, #2
	cmp r4, r3
	blt main$L105
main$L106:
main$L100110:
	add r2, r0, r2
	mov r0, r2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
main$L100111:
	mov r2, #1
	mov r2, r2
	b main$L100110
main$L100108:
	mov r2, r1
	b main$L100111

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
