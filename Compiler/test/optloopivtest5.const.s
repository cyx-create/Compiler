.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, t0
movw t123, #0
cmp t10000, t123
bgt L107
L108:
mov t117, #4
mov t10104, t117
L109:
L100110:
mov r0, t10104
bl putint
movw t122, #10
mov r0, t122
bl putch
mov r0, t10104
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L107:
mov t115, #10
mov t116, #0
mov t10201, t115
mov t10101, t116
L104:
L100111:
movw t124, #0
cmp t10201, t124
bgt L105
L106:
mov t10104, t10101
b L100110
L105:
movw t120, #3
mul t11000, t120, t10201
add t10102, t11000, #2
mov r0, t10102
bl putint
movw t121, #32
mov r0, t121
bl putch
sub t10202, t10201, #1
mov t10201, t10202
mov t10101, t10102
b L100111

