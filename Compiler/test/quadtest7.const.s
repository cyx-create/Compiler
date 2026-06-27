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

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t121, #12
mov r0, t121
bl malloc
mov t10300, t0
adr t122, A^f
str t122, [t10300, #8]
mov t10001, t10300
movw t123, #1
str t123, [t10001]
movw t124, #12
mov r0, t124
bl malloc
mov t10400, t0
adr t125, B^f
str t125, [t10400, #8]
mov t10101, t10400
movw t126, #10
str t126, [t10101, #4]
ldr t11600, [t10001]
ldr t11800, [t10101, #4]
add t10700, t11600, t11800
mov r0, t10700
bl putint
movw t127, #10
mov r0, t127
bl putch
movw t128, #0
mov r0, t128
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

