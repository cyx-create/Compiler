.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t107, #16
mov r0, t107
bl malloc
mov t10000, t0
movw t108, #3
str t108, [t10000]
movw t109, #1
str t109, [t10000, #4]
movw t110, #2
str t110, [t10000, #8]
movw t111, #3
str t111, [t10000, #12]
movw t112, #0
mov r0, t112
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

