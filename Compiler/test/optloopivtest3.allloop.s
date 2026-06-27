.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10200, #0
bl getint
mov t10300, t0
bl getint
mov t10400, t0
sub t10206, t10300, t10400
movw t117, #8
mul t10207, t10206, t117
add t10204, t10207, #7
movw t118, #8
mul t10208, t10400, t118
mov t10201, t10200
mov t10203, t10204
L102:
L100105:
movw t121, #7
cmp t10203, t121
bgt L103
L104:
movw t120, #10
mov r0, t120
bl putch
mov r0, t10201
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov r0, t10203
bl putint
movw t119, #32
mov r0, t119
bl putch
sub t10205, t10208, t10208
mov t10201, t10203
mov t10203, t10205
b L100105

