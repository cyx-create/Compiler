.global C^m
C^m:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
ldr t10300, [t100]
mov r0, t10300
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t135, #8
mov r0, t135
bl malloc
mov t10400, t0
movw t136, #20
mov r0, t136
bl malloc
mov t10500, t0
movw t137, #4
str t137, [t10500]
movw t138, #1
str t138, [t10500, #4]
movw t139, #2
str t139, [t10500, #8]
movw t140, #3
str t140, [t10500, #12]
movw t141, #4
str t141, [t10500, #16]
mov t12400, t10400
str t10500, [t12400]
add t12500, t10400, #4
adr t142, C^m
str t142, [t12500]
mov t10600, t10400
ldr t11000, [t10600, #4]
mov r12, t11000
mov r0, t10600
blx r12
mov t11100, t0
mov t10000, t11100
mov t10200, #3
mov t10201, t10200
L102:
L100108:
movw t146, #0
cmp t10201, t146
bge L103
L104:
movw t145, #10
mov r0, t145
bl putch
movw t147, #2
mov r0, t147
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10202, t10201, #1
mov t11300, t10000
ldr t10700, [t10000]
movw t148, #0
cmp t10202, t148
bge L106
L105:
movw t149, #65535
movt t149, #65535
mov r0, t149
bl exit
L106:
cmp t10202, t10700
bge L105
L107:
add t12900, t10202, #1
movw t143, #4
mul t13000, t12900, t143
ldr t11400, [t11300, t13000]
mov r0, t11400
bl putint
movw t144, #32
mov r0, t144
bl putch
mov t10201, t10202
b L100108

