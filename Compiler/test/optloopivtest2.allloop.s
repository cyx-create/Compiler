.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, t0
movw t114, #4
mul t10304, t10000, t114
add t10302, t10304, #2
mov t10001, t10000
mov t10301, t10302
L102:
L100105:
movw t117, #2
cmp t10301, t117
bgt L103
L104:
mov r0, t10001
bl putint
movw t116, #10
mov r0, t116
bl putch
movw t118, #2
mov r0, t118
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov r0, t10301
bl putint
movw t115, #32
mov r0, t115
bl putch
sub t10002, t10001, #2
sub t10303, t10301, #8
mov t10001, t10002
mov t10301, t10303
b L100105

