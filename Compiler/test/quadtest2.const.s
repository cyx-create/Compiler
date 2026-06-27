.global __$main__^main
__$main__^main:
L113:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t146, #4
mov r0, t146
bl malloc
mov t10400, t0
adr t147, fib^f
str t147, [t10400]
mov t10201, t10400
movw t148, #69
mov r0, t148
bl putch
movw t149, #110
mov r0, t149
bl putch
movw t150, #116
mov r0, t150
bl putch
movw t151, #101
mov r0, t151
bl putch
movw t152, #114
mov r0, t152
bl putch
movw t153, #32
mov r0, t153
bl putch
movw t154, #116
mov r0, t154
bl putch
movw t155, #104
mov r0, t155
bl putch
movw t156, #101
mov r0, t156
bl putch
movw t157, #32
mov r0, t157
bl putch
movw t158, #110
mov r0, t158
bl putch
movw t159, #117
mov r0, t159
bl putch
movw t160, #109
mov r0, t160
bl putch
movw t161, #98
mov r0, t161
bl putch
movw t162, #101
mov r0, t162
bl putch
movw t163, #114
mov r0, t163
bl putch
movw t164, #32
mov r0, t164
bl putch
movw t165, #111
mov r0, t165
bl putch
movw t166, #102
mov r0, t166
bl putch
movw t167, #32
mov r0, t167
bl putch
movw t168, #116
mov r0, t168
bl putch
movw t169, #101
mov r0, t169
bl putch
movw t170, #114
mov r0, t170
bl putch
movw t171, #109
mov r0, t171
bl putch
movw t172, #58
mov r0, t172
bl putch
bl getint
mov t10100, t0
movw t175, #0
cmp t10100, t175
blt L100113
L104:
movw t176, #47
cmp t10100, t176
bgt L105
L106:
L107:
mov t143, #0
mov t10003, t143
L110:
L100114:
cmp t10003, t10100
blt L111
L112:
mov t10004, t10003
movw t174, #10
mov r0, t174
bl putch
movw t177, #0
mov r0, t177
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
mov t10004, t10003
ldr t13200, [t10201]
mov r12, t13200
mov r0, t10201
mov r1, t10004
blx r12
mov t13300, t0
mov r0, t13300
bl putint
movw t173, #32
mov r0, t173
bl putch
add t10005, t10004, #1
mov t10003, t10005
b L100114
L105:
movw t178, #65535
movt t178, #65535
mov r0, t178
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100113:
b L105

.global fib^f
fib^f:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
mov t101, t1
movw t120, #0
cmp t101, t120
beq L100108
L104:
movw t121, #1
cmp t101, t121
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

