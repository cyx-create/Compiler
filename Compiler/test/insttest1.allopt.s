.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t126, #20
mov r0, t126
bl malloc
mov t10000, t0
movw t127, #4
str t127, [t10000]
movw t128, #1
str t128, [t10000, #4]
movw t129, #2
str t129, [t10000, #8]
movw t130, #3
str t130, [t10000, #12]
movw t131, #4
str t131, [t10000, #16]
ldr t10300, [t10000]
mov t10100, t10300
mov t10700, t10000
mov t10101, t10100
L102:
L100108:
movw t135, #0
cmp t10101, t135
bgt L103
L104:
movw t134, #10
mov r0, t134
bl putch
movw t136, #2
mov r0, t136
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
ldr t10400, [t10000]
movw t137, #0
cmp t10102, t137
bge L106
L105:
movw t138, #65535
movt t138, #65535
mov r0, t138
bl exit
L106:
cmp t10102, t10400
bge L105
L107:
add t11800, t10102, #1
movw t132, #4
mul t11900, t11800, t132
ldr t10800, [t10700, t11900]
mov r0, t10800
bl putint
movw t133, #32
mov r0, t133
bl putch
mov t10101, t10102
b L100108

