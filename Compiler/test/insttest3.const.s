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
movw t137, #8
mov r0, t137
bl malloc
mov t10400, t0
movw t138, #20
mov r0, t138
bl malloc
mov t10500, t0
movw t139, #4
str t139, [t10500]
movw t140, #1
str t140, [t10500, #4]
movw t141, #2
str t141, [t10500, #8]
movw t142, #3
str t142, [t10500, #12]
movw t143, #4
str t143, [t10500, #16]
mov t12400, t10400
str t10500, [t12400]
add t12500, t10400, #4
adr t144, C^m
str t144, [t12500]
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
movw t148, #0
cmp t10201, t148
bge L103
L104:
movw t147, #10
mov r0, t147
bl putch
movw t149, #2
mov r0, t149
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10202, t10201, #1
mov t11300, t10000
ldr t10700, [t10000]
movw t150, #0
cmp t10202, t150
bge L106
L105:
movw t151, #65535
movt t151, #65535
mov r0, t151
bl exit
L106:
cmp t10202, t10700
bge L105
L107:
add t12900, t10202, #1
movw t145, #4
mul t13000, t12900, t145
ldr t11400, [t11300, t13000]
mov r0, t11400
bl putint
movw t146, #32
mov r0, t146
bl putch
mov t10201, t10202
b L100108

