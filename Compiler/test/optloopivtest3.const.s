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
mov t10301, t10300
mov t10201, t10200
L102:
L100105:
movw t122, #0
cmp t10301, t122
bgt L103
L104:
movw t121, #10
mov r0, t121
bl putch
mov r0, t10201
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10302, t10301, t10400
movw t119, #8
mul t11300, t119, t10302
add t10202, t11300, #7
mov r0, t10202
bl putint
movw t120, #32
mov r0, t120
bl putch
mov t10301, t10302
mov t10201, t10202
b L100105

