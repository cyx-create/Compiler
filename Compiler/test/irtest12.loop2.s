.global __$main__^main
__$main__^main:
L106:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t120, #16
mov r0, t120
bl malloc
mov t10000, t0
movw t121, #3
str t121, [t10000]
movw t122, #1
str t122, [t10000, #4]
movw t123, #2
str t123, [t10000, #8]
movw t124, #3
str t124, [t10000, #12]
movw t125, #0
movw t126, #1
sub t10100, t125, t126
mov t10700, t10000
ldr t10300, [t10000]
movw t129, #0
cmp t10100, t129
bge L101
L100:
mov t10101, t10100
L100106:
movw t130, #65535
movt t130, #65535
mov r0, t130
bl exit
L101:
mov t10101, t10100
L100107:
cmp t10101, t10300
bge L100
L102:
mov t10102, t10101
L100108:
add t11400, t10102, #1
movw t127, #4
mul t11500, t11400, t127
movw t128, #4
str t128, [t10700, t11500]
ldr t10400, [t10000]
movw t131, #2
movw t132, #0
cmp t131, t132
bge L104
L103:
movw t133, #65535
movt t133, #65535
mov r0, t133
bl exit
L104:
movw t134, #2
cmp t134, t10400
bge L103
L105:
movw t135, #0
mov r0, t135
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

