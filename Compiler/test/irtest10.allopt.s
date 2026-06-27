.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t109, #16
mov r0, t109
bl malloc
mov t10000, t0
movw t110, #3
str t110, [t10000]
movw t111, #1
str t111, [t10000, #4]
movw t112, #2
str t112, [t10000, #8]
movw t113, #3
str t113, [t10000, #12]
movw t114, #0
mov r0, t114
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

