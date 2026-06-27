.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t121, #24
mov r0, t121
bl malloc
mov t10000, t0
movw t122, #5
str t122, [t10000]
movw t123, #1
str t123, [t10000, #4]
movw t124, #2
str t124, [t10000, #8]
movw t125, #3
str t125, [t10000, #12]
movw t126, #4
str t126, [t10000, #16]
movw t127, #5
str t127, [t10000, #20]
mov t10100, #0
mov t10800, t10000
mov t10101, t10100
L102:
L100108:
mov t10600, t10101
ldr t10300, [t10000]
cmp t10600, t10300
blt L103
L104:
mov t10102, t10101
L100109:
movw t130, #1
mov r0, t130
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov t10102, t10101
L100110:
ldr t10400, [t10000]
movw t131, #0
cmp t10102, t131
bge L106
L105:
mov t10103, t10102
L100111:
movw t132, #65535
movt t132, #65535
mov r0, t132
bl exit
L106:
mov t10103, t10102
L100112:
cmp t10103, t10400
bge L105
L107:
mov t10104, t10103
L100113:
add t11800, t10104, #1
movw t128, #4
mul t11900, t11800, t128
ldr t10900, [t10800, t11900]
mov r0, t10900
bl putint
movw t129, #10
mov r0, t129
bl putch
add t10105, t10104, #1
mov t10101, t10105
b L100108

