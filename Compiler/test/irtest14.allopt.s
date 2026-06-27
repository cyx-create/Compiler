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
movw t109, #8
mov r0, t109
bl malloc
mov t10200, t0
adr t110, A^f
str t110, [t10200, #4]
mov t10001, t10200
movw t111, #1
str t111, [t10001]
movw t112, #0
mov r0, t112
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

