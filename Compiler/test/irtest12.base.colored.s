.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #16
	mov r0, r0
	bl malloc
	mov r4, r0
	movw r0, #3
	str r0, [r4]
	movw r0, #1
	str r0, [r4, #4]
	movw r0, #2
	str r0, [r4, #8]
	movw r0, #3
	str r0, [r4, #12]
	movw r1, #0
	movw r0, #1
	sub r6, r1, r0
	ldr r5, [r4]
	movw r0, #0
	cmp r6, r0
	bge main$L101
main$L100:
	mov r0, r6
main$L100106:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L101:
	mov r0, r6
main$L100107:
	cmp r0, r5
	bge main$L100
main$L102:
	mov r5, r0
main$L100108:
	add r0, r5, #1
	movw r1, #4
	mul r0, r0, r1
	movw r1, #4
	str r1, [r4, r0]
	ldr r6, [r4]
	movw r1, #2
	movw r0, #0
	cmp r1, r0
	bge main$L104
main$L103:
	mov r0, r5
main$L100109:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L104:
	mov r0, r5
main$L100110:
	movw r0, #2
	cmp r0, r6
	bge main$L103
main$L105:
	mov r0, #3
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
