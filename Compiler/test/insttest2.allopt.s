.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t132, #4
mov r0, t132
bl malloc
mov t10300, t0
movw t133, #20
mov r0, t133
bl malloc
mov t10400, t0
movw t134, #4
str t134, [t10400]
movw t135, #1
str t135, [t10400, #4]
movw t136, #2
str t136, [t10400, #8]
movw t137, #3
str t137, [t10400, #12]
movw t138, #4
str t138, [t10400, #16]
mov t12100, t10300
str t10400, [t12100]
mov t10000, t10300
mov t10100, #3
mov t10101, t10100
L102:
L100108:
movw t142, #0
cmp t10101, t142
bge L103
L104:
movw t141, #10
mov r0, t141
bl putch
movw t143, #2
mov r0, t143
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
ldr t10500, [t12200]
mov t11000, t10500
ldr t10600, [t10500]
movw t144, #0
cmp t10102, t144
bge L106
L105:
movw t145, #65535
movt t145, #65535
mov r0, t145
bl exit
L106:
cmp t10102, t10600
bge L105
L107:
add t12400, t10102, #1
movw t139, #4
mul t12500, t12400, t139
ldr t11100, [t11000, t12500]
mov r0, t11100
bl putint
movw t140, #32
mov r0, t140
bl putch
mov t10101, t10102
b L100108

