.global C^m
C^m:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
mov t101, t1
movw t109, #0
cmp t101, t109
bgt L102
L103:
ldr t10600, [t100, #4]
mov r0, t10600
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L102:
ldr t10400, [t100]
mov r0, t10400
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
movw t150, #12
mov r0, t150
bl malloc
mov t10300, t0
movw t151, #20
mov r0, t151
bl malloc
mov t10400, t0
movw t152, #4
str t152, [t10400]
movw t153, #1
str t153, [t10400, #4]
movw t154, #2
str t154, [t10400, #8]
movw t155, #3
str t155, [t10400, #12]
movw t156, #4
str t156, [t10400, #16]
mov t12900, t10300
str t10400, [t12900]
movw t157, #24
mov r0, t157
bl malloc
mov t10500, t0
movw t158, #5
str t158, [t10500]
movw t159, #5
str t159, [t10500, #4]
movw t160, #6
str t160, [t10500, #8]
movw t161, #7
str t161, [t10500, #12]
movw t162, #8
str t162, [t10500, #16]
movw t163, #9
str t163, [t10500, #20]
add t13600, t10300, #4
str t10500, [t13600]
add t13700, t10300, #8
adr t164, C^m
str t164, [t13700]
mov t10600, t10300
add t13800, t10600, #8
ldr t11200, [t13800]
mov t11500, t11200
mov t11400, t10600
bl getint
mov t11300, t0
mov r12, t11500
mov r0, t11400
mov r1, t11300
blx r12
mov t11600, t0
mov t10000, t11600
ldr t10700, [t10000]
mov t10100, t10700
mov t11800, t10000
mov t10101, t10100
L102:
L100108:
movw t168, #0
cmp t10101, t168
bge L103
L104:
movw t167, #10
mov r0, t167
bl putch
movw t169, #2
mov r0, t169
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
ldr t10800, [t10000]
movw t170, #0
cmp t10102, t170
bge L106
L105:
movw t171, #65535
movt t171, #65535
mov r0, t171
bl exit
L106:
cmp t10102, t10800
bge L105
L107:
add t14200, t10102, #1
movw t165, #4
mul t14300, t14200, t165
ldr t11900, [t11800, t14300]
mov r0, t11900
bl putint
movw t166, #32
mov r0, t166
bl putch
mov t10101, t10102
b L100108

