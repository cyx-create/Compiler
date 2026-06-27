.global __$main__^main
__$main__^main:
L106:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t122, #16
mov r0, t122
bl malloc
mov t10000, t0
movw t123, #3
str t123, [t10000]
movw t124, #1
str t124, [t10000, #4]
movw t125, #2
str t125, [t10000, #8]
movw t126, #3
str t126, [t10000, #12]
mov t10700, t10000
ldr t10300, [t10000]
L100:
movw t127, #65535
movt t127, #65535
mov r0, t127
bl exit

