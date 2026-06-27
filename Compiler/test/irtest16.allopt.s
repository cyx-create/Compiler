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

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t114, #12
mov r0, t114
bl malloc
mov t10300, t0
adr t115, A^f
str t115, [t10300, #8]
mov t10001, t10300
movw t116, #1
str t116, [t10001]
movw t117, #12
mov r0, t117
bl malloc
mov t10400, t0
adr t118, B^f
str t118, [t10400, #8]
mov t10101, t10400
movw t119, #1
str t119, [t10101, #4]
movw t120, #0
mov r0, t120
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

