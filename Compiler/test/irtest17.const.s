.global B^g
B^g:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #0
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t119, #16
mov r0, t119
bl malloc
mov t10300, t0
adr t120, A^f
str t120, [t10300, #8]
mov t10001, t10300
movw t121, #1
str t121, [t10001]
movw t122, #16
mov r0, t122
bl malloc
mov t10400, t0
adr t123, B^f
str t123, [t10400, #8]
adr t124, B^g
str t124, [t10400, #12]
mov t10101, t10400
movw t125, #1
str t125, [t10101, #4]
mov t10002, t10101
add t11600, t10002, #8
ldr t10700, [t11600]
mov r12, t10700
mov r0, t10002
blx r12
mov t10800, t0
mov r0, t10800
bl putint
movw t126, #0
mov r0, t126
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #0
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #0
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

