.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, t0
sub t10304, t10000, #1
movw t112, #4
mul t10305, t10304, t112
add t10302, t10305, #2
mov t10301, t10302
L102:
L100105:
movw t115, #65534
cmp t10301, t115
bgt L103
L104:
movw t114, #10
mov r0, t114
bl putch
movw t116, #2
mov r0, t116
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov r0, t10301
bl putint
movw t113, #32
mov r0, t113
bl putch
sub t10303, t10301, #4
mov t10301, t10303
b L100105

