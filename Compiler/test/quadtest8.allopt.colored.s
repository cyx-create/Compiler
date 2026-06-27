.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #16
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =A$f
	str r1, [r0, #8]
	movw r1, #4
	str r1, [r0]
	movw r0, #16
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =B$f
	str r1, [r0, #8]
	ldr r1, =B$g
	str r1, [r0, #12]
	mov r4, r0
	movw r0, #2
	str r0, [r4, #4]
	mov r0, r4
	add r1, r0, #8
	ldr r1, [r1]
	mov r12, r1
	mov r0, r0
	blx r12
	mov r0, r0
	ldr r1, [r4, #4]
	add r0, r0, r1
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	movw r0, #100
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	movw r0, #200
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	movw r0, #300
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
