.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10100, #10
sub t10004, t10100, #1
movw t114, #4
mul t10005, t10004, t114
sub t10002, t10005, #2
mov t10101, t10100
mov t10001, t10002
L102:
L100105:
movw t117, #65530
cmp t10001, t117
bgt L103
L104:
mov r0, t10101
bl putint
movw t116, #10
mov r0, t116
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
movw t115, #32
mov r0, t115
bl putch
sub t10003, t10001, #4
mov t10101, t10102
mov t10001, t10003
b L100105

