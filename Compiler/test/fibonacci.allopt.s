.global __$main__^main
__$main__^main:
L113:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t174, #4
mov r0, t174
bl malloc
mov t10400, t0
adr t175, fib^f
str t175, [t10400]
mov t10201, t10400
movw t176, #69
mov r0, t176
bl putch
movw t177, #110
mov r0, t177
bl putch
movw t178, #116
mov r0, t178
bl putch
movw t179, #101
mov r0, t179
bl putch
movw t180, #114
mov r0, t180
bl putch
movw t181, #32
mov r0, t181
bl putch
movw t182, #116
mov r0, t182
bl putch
movw t183, #104
mov r0, t183
bl putch
movw t184, #101
mov r0, t184
bl putch
movw t185, #32
mov r0, t185
bl putch
movw t186, #110
mov r0, t186
bl putch
movw t187, #117
mov r0, t187
bl putch
movw t188, #109
mov r0, t188
bl putch
movw t189, #98
mov r0, t189
bl putch
movw t190, #101
mov r0, t190
bl putch
movw t191, #114
mov r0, t191
bl putch
movw t192, #32
mov r0, t192
bl putch
movw t193, #111
mov r0, t193
bl putch
movw t194, #102
mov r0, t194
bl putch
movw t195, #32
mov r0, t195
bl putch
movw t196, #116
mov r0, t196
bl putch
movw t197, #101
mov r0, t197
bl putch
movw t198, #114
mov r0, t198
bl putch
movw t199, #109
mov r0, t199
bl putch
movw t200, #58
mov r0, t200
bl putch
bl getint
mov t10100, t0
movw t203, #0
cmp t10100, t203
blt L100113
L104:
movw t204, #47
cmp t10100, t204
bgt L105
L106:
L107:
mov t10001, t171
L110:
L100114:
cmp t10001, t10100
blt L111
L112:
movw t202, #10
mov r0, t202
bl putch
movw t205, #0
mov r0, t205
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L111:
ldr t13200, [t16600]
mov r12, t13200
mov r0, t10201
mov r1, t10001
blx r12
mov t13300, t0
mov r0, t13300
bl putint
movw t201, #32
mov r0, t201
bl putch
add t10002, t10001, #1
mov t10001, t10002
b L100114
L105:
movw t206, #65535
movt t206, #65535
mov r0, t206
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
movw t121, #0
cmp t101, t121
beq L100108
L104:
movw t122, #1
cmp t101, t122
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
add t11800, t11300, t11200
mov r0, t11800
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

