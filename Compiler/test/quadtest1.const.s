.global b1^bubbleSort
b1^bubbleSort:
L133:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t101, t0
mov t102, t1
mov t103, t2
movw t169, #1
cmp t103, t169
ble L102
L103:
L104:
mov t160, #0
mov t10002, t160
L107:
L100133:
sub t13400, t103, #1
cmp t10002, t13400
blt L108
L109:
mov t10003, t10002
ldr t12700, [t101, #4]
mov t13100, t12700
mov t13000, t101
mov t12900, t102
sub t12800, t103, #1
mov r12, t13100
mov r0, t13000
mov r1, t12900
mov r2, t12800
blx r12
mov t13200, t0
mov r0, t13200
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L108:
mov t10003, t10002
mov t11500, t102
ldr t10500, [t102]
movw t170, #0
cmp t10003, t170
bge L111
L110:
mov t10004, t10003
L100134:
movw t171, #65535
movt t171, #65535
mov r0, t171
bl exit
L111:
mov t10004, t10003
cmp t10004, t10500
bge L110
L112:
mov t10005, t10004
add t13500, t10005, #1
movw t163, #4
mul t13600, t13500, t163
ldr t11800, [t11500, t13600]
add t10600, t10005, #1
mov t11700, t102
ldr t10700, [t102]
movw t172, #0
cmp t10600, t172
bge L114
L113:
mov t10006, t10005
L100135:
movw t173, #65535
movt t173, #65535
mov r0, t173
bl exit
L114:
mov t10006, t10005
cmp t10600, t10700
bge L113
L115:
mov t10007, t10006
add t13800, t10600, #1
movw t164, #4
mul t13900, t13800, t164
ldr t14100, [t11700, t13900]
cmp t11800, t14100
bgt L130
L131:
mov t10008, t10007
L132:
L100136:
add t10009, t10008, #1
mov t10002, t10009
b L100133
L130:
mov t10008, t10007
mov t12000, t102
ldr t10800, [t102]
movw t174, #0
cmp t10008, t174
bge L119
L118:
mov t10009, t10008
L100137:
movw t175, #65535
movt t175, #65535
mov r0, t175
bl exit
L119:
mov t10009, t10008
cmp t10009, t10800
bge L118
L120:
mov t10010, t10009
mov t14200, t101
add t14300, t10010, #1
movw t165, #4
mul t14400, t14300, t165
ldr t14600, [t12000, t14400]
str t14600, [t14200]
mov t12200, t102
ldr t11100, [t102]
movw t176, #0
cmp t10010, t176
bge L125
L124:
mov t10011, t10010
L100138:
movw t177, #65535
movt t177, #65535
mov r0, t177
bl exit
L125:
mov t10011, t10010
cmp t10011, t11100
bge L124
L126:
mov t10012, t10011
add t10900, t10012, #1
mov t12400, t102
ldr t11000, [t102]
movw t178, #0
cmp t10900, t178
bge L122
L121:
mov t10013, t10012
L100139:
movw t179, #65535
movt t179, #65535
mov r0, t179
bl exit
L122:
mov t10013, t10012
cmp t10900, t11000
bge L121
L123:
mov t10014, t10013
add t14700, t10014, #1
movw t166, #4
mul t14800, t14700, t166
add t14900, t12200, t14800
add t15000, t10900, #1
movw t167, #4
mul t15100, t15000, t167
ldr t15300, [t12400, t15100]
str t15300, [t14900]
add t11200, t10014, #1
mov t12600, t102
ldr t11300, [t102]
movw t180, #0
cmp t11200, t180
bge L128
L127:
mov t10015, t10014
L100140:
movw t181, #65535
movt t181, #65535
mov r0, t181
bl exit
L128:
mov t10015, t10014
cmp t11200, t11300
bge L127
L129:
mov t10016, t10015
add t15400, t11200, #1
movw t168, #4
mul t15500, t15400, t168
ldr t15800, [t101]
str t15800, [t12600, t15500]
mov t10008, t10016
b L100136
L102:
movw t182, #0
mov r0, t182
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
movw t137, #32
mov r0, t137
bl malloc
mov t10000, t0
movw t138, #7
str t138, [t10000]
movw t139, #6
str t139, [t10000, #4]
movw t140, #3
str t140, [t10000, #8]
movw t141, #0
str t141, [t10000, #12]
movw t142, #5
str t142, [t10000, #16]
movw t143, #9
str t143, [t10000, #20]
movw t144, #1
str t144, [t10000, #24]
movw t145, #2
str t145, [t10000, #28]
mov t10200, #0
movw t146, #8
mov r0, t146
bl malloc
mov t10400, t0
adr t147, b1^bubbleSort
str t147, [t10400, #4]
mov t10101, t10400
ldr t11000, [t10101, #4]
mov t11300, t11000
mov t11200, t10101
mov t11100, t10000
ldr t10500, [t10000]
mov r12, t11300
mov r0, t11200
mov r1, t11100
mov r2, t10500
blx r12
mov t10201, t10200
L102:
L100108:
mov t11500, t10201
ldr t10600, [t10000]
cmp t11500, t10600
blt L103
L104:
mov t10202, t10201
movw t150, #10
mov r0, t150
bl putch
movw t151, #0
mov r0, t151
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov t10202, t10201
mov t11700, t10000
ldr t10700, [t10000]
movw t152, #0
cmp t10202, t152
bge L106
L105:
mov t10203, t10202
L100109:
movw t153, #65535
movt t153, #65535
mov r0, t153
bl exit
L106:
mov t10203, t10202
cmp t10203, t10700
bge L105
L107:
mov t10204, t10203
add t13200, t10204, #1
movw t148, #4
mul t13300, t13200, t148
ldr t11800, [t11700, t13300]
mov r0, t11800
bl putint
movw t149, #32
mov r0, t149
bl putch
add t10205, t10204, #1
mov t10201, t10205
b L100108

