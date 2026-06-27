.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, t0
mov t10200, #10
mov t10100, #0
mov t10201, t10200
mov t10101, t10100
L102:
L100110:
movw t120, #0
cmp t10201, t120
bgt L103
L104:
mov r0, t10101
bl putint
movw t119, #10
mov r0, t119
bl putch
mov r0, t10101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t121, #0
cmp t10000, t121
bgt L107
L108:
mov t10103, #4
mov t10104, t10103
L109:
L100111:
sub t10202, t10201, #1
mov t10201, t10202
mov t10101, t10104
b L100110
L107:
movw t117, #3
mul t11000, t117, t10201
add t10102, t11000, #2
mov r0, t10102
bl putint
movw t118, #32
mov r0, t118
bl putch
mov t10104, t10102
b L100111

