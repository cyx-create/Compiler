.global __$main__^main
__$main__^main:
L113:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #0
movw t141, #4
mov r0, t141
bl malloc
mov t10400, t0
adr t142, fib^f
str t142, [t10400]
mov t10201, t10400
movw t143, #69
mov r0, t143
bl putch
movw t144, #110
mov r0, t144
bl putch
movw t145, #116
mov r0, t145
bl putch
movw t146, #101
mov r0, t146
bl putch
movw t147, #114
mov r0, t147
bl putch
movw t148, #32
mov r0, t148
bl putch
movw t149, #116
mov r0, t149
bl putch
movw t150, #104
mov r0, t150
bl putch
movw t151, #101
mov r0, t151
bl putch
movw t152, #32
mov r0, t152
bl putch
movw t153, #110
mov r0, t153
bl putch
movw t154, #117
mov r0, t154
bl putch
movw t155, #109
mov r0, t155
bl putch
movw t156, #98
mov r0, t156
bl putch
movw t157, #101
mov r0, t157
bl putch
movw t158, #114
mov r0, t158
bl putch
movw t159, #32
mov r0, t159
bl putch
movw t160, #111
mov r0, t160
bl putch
movw t161, #102
mov r0, t161
bl putch
movw t162, #32
mov r0, t162
bl putch
movw t163, #116
mov r0, t163
bl putch
movw t164, #101
mov r0, t164
bl putch
movw t165, #114
mov r0, t165
bl putch
movw t166, #109
mov r0, t166
bl putch
movw t167, #58
mov r0, t167
bl putch
bl getint
mov t10100, t0
movw t172, #0
cmp t10100, t172
blt L100113
L104:
mov t10001, t10000
L100114:
movw t173, #47
cmp t10100, t173
bgt L105
L106:
mov t10002, t10001
L100115:
L107:
add t10005, t10004, #1
mov t10003, t10002
L110:
L100116:
cmp t10003, t10100
blt L111
L112:
mov t10004, t10003
L100117:
movw t171, #10
mov r0, t171
bl putch
movw t174, #0
mov r0, t174
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
mov t10004, t10003
L100118:
ldr t13200, [t14000]
mov r12, t13200
mov r0, t10201
mov r1, t10004
blx r12
mov t13300, t0
mov r0, t13300
bl putint
movw t170, #32
mov r0, t170
bl putch
mov t10003, t10005
b L100116
L105:
L100119:
movw t168, #0
movw t169, #1
sub t13900, t168, t169
mov r0, t13900
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100113:
mov t10001, t10000
b L100119

.global fib^f
fib^f:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
mov t101, t1
movw t118, #0
cmp t101, t118
beq L100108
L104:
movw t119, #1
cmp t101, t119
beq L105
L106:
ldr t10300, [t100]
mov t10600, t10300
mov t10500, t100
sub t10400, t101, #1
mov r12, t10600
mov r0, t10500
mov r1, t10400
blx r12
mov t10700, t0
mov t11300, t10700
mov t11600, t100
ldr t10800, [t11600]
mov t11100, t10800
mov t11000, t100
sub t10900, t101, #2
mov r12, t11100
mov r0, t11000
mov r1, t10900
blx r12
mov t11200, t0
add t11700, t11300, t11200
mov r0, t11700
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L105:
mov r0, t101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100108:
b L105

