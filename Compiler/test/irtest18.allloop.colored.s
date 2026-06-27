.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, r0
	add r0, r1, #12
	ldr r0, [r0]
	mov r12, r0
	mov r0, r1
	blx r12
	mov r0, r0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #20
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =A$f
	str r1, [r0, #12]
	mov r5, r0
	movw r0, #1
	str r0, [r5]
	movw r0, #20
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =B$f
	str r1, [r0, #12]
	ldr r1, =B$g
	str r1, [r0, #16]
	mov r4, r0
	movw r0, #1
	str r0, [r4, #8]
	add r0, r5, #12
	ldr r0, [r0]
	mov r12, r0
	mov r0, r5
	blx r12
	mov r0, r0
	mov r0, r0
	bl putint
	mov r0, r4
	add r1, r0, #12
	ldr r1, [r1]
	mov r12, r1
	mov r0, r0
	blx r12
	mov r0, r0
	mov r0, r0
	bl putint
	movw r0, #0
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
B$f$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, r0
	mov r0, #0
	ldr r1, [r0]
	add r0, r6, #8
	ldr r0, [r0]
	add r5, r1, r0
	ldr r0, [r6, #4]
	mov r4, r0
	ldr r7, [r0]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge B$f$L101
B$f$L100:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
B$f$L101:
	movw r0, #0
	cmp r0, r7
	bge B$f$L100
B$f$L102:
	mov r0, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	add r4, r5, r0
	add r0, r6, #16
	ldr r0, [r0]
	mov r12, r0
	mov r0, r6
	blx r12
	mov r0, r0
	add r0, r4, r0
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
