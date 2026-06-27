.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10100, #10
sub t10004, t10100, #1
movw t112, #4
mul t10005, t10004, t112
sub t10002, t10005, #2
mov t10101, t10100
mov t10001, t10002
L102:
L100105:
movw t115, #65530
cmp t10001, t115
bgt L103
L104:
mov r0, t10101
bl putint
movw t114, #10
mov r0, t114
bl putch
mov r0, t10101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
mov r0, t10001
bl putint
movw t113, #32
mov r0, t113
bl putch
sub t10003, t10001, #4
mov t10101, t10102
mov t10001, t10003
b L100105

