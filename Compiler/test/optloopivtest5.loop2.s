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
movw t118, #0
cmp t10000, t118
bgt L107
L108:
mov t10103, #4
mov t10104, t10103
L109:
L100110:
mov r0, t10104
bl putint
movw t117, #10
mov r0, t117
bl putch
mov r0, t10104
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L107:
movw t115, #3
mul t10108, t10200, t115
add t10106, t10108, #2
mov t10101, t10100
mov t10105, t10106
L104:
L100111:
movw t119, #2
cmp t10105, t119
bgt L105
L106:
mov t10104, t10101
b L100110
L105:
mov r0, t10105
bl putint
movw t116, #32
mov r0, t116
bl putch
sub t10107, t10105, #3
mov t10101, t10105
mov t10105, t10107
b L100111

