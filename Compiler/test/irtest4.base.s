.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10001, t10000
L105:
L100108:
movw t106, #0
cmp t10001, t106
bgt L100109
L104:
mov t10002, t10001
L100110:
movw t107, #0
cmp t10002, t107
beq L106
L107:
mov t10003, t10002
L100111:
mov r0, t10003
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L106:
L100112:
mov r0, t10002
bl putint
movw t105, #10
mov r0, t105
bl putch
sub t10003, t10002, #1
mov t10001, t10003
b L100108
L100109:
mov t10002, t10001
b L100112

