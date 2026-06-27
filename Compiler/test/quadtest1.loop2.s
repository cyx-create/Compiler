.global b1^bubbleSort
b1^bubbleSort:
L133:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t101, t0
mov t102, t1
mov t103, t2
mov t10000, #0
movw t166, #1
cmp t103, t166
ble L102
L103:
mov t10001, t10000
L100133:
L104:
mov t10002, t10001
L107:
L100134:
sub t13400, t103, #1
cmp t10002, t13400
blt L108
L109:
mov t10003, t10002
L100135:
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
L100136:
mov t11500, t102
ldr t10500, [t102]
movw t167, #0
cmp t10003, t167
bge L111
L110:
mov t10004, t10003
L100137:
movw t168, #65535
movt t168, #65535
mov r0, t168
bl exit
L111:
mov t10004, t10003
L100138:
cmp t10004, t10500
bge L110
L112:
mov t10005, t10004
L100139:
add t13500, t10005, #1
movw t160, #4
mul t13600, t13500, t160
ldr t11800, [t11500, t13600]
add t10600, t10005, #1
mov t11700, t102
ldr t10700, [t102]
movw t169, #0
cmp t10600, t169
bge L114
L113:
mov t10006, t10005
L100140:
movw t170, #65535
movt t170, #65535
mov r0, t170
bl exit
L114:
mov t10006, t10005
L100141:
cmp t10600, t10700
bge L113
L115:
mov t10007, t10006
L100142:
add t13800, t10600, #1
movw t161, #4
mul t13900, t13800, t161
ldr t14100, [t11700, t13900]
cmp t11800, t14100
bgt L130
L131:
mov t10008, t10007
L100143:
L132:
L100144:
add t10009, t10008, #1
mov t10002, t10009
b L100134
L130:
mov t10008, t10007
L100145:
mov t12000, t102
ldr t10800, [t102]
movw t171, #0
cmp t10008, t171
bge L119
L118:
mov t10009, t10008
L100146:
movw t172, #65535
movt t172, #65535
mov r0, t172
bl exit
L119:
mov t10009, t10008
L100147:
cmp t10009, t10800
bge L118
L120:
mov t10010, t10009
L100148:
mov t14200, t101
add t14300, t10010, #1
movw t162, #4
mul t14400, t14300, t162
ldr t14600, [t12000, t14400]
str t14600, [t14200]
mov t12200, t102
ldr t11100, [t102]
movw t173, #0
cmp t10010, t173
bge L125
L124:
mov t10011, t10010
L100149:
movw t174, #65535
movt t174, #65535
mov r0, t174
bl exit
L125:
mov t10011, t10010
L100150:
cmp t10011, t11100
bge L124
L126:
mov t10012, t10011
L100151:
add t10900, t10012, #1
mov t12400, t102
ldr t11000, [t102]
movw t175, #0
cmp t10900, t175
bge L122
L121:
mov t10013, t10012
L100152:
movw t176, #65535
movt t176, #65535
mov r0, t176
bl exit
L122:
mov t10013, t10012
L100153:
cmp t10900, t11000
bge L121
L123:
mov t10014, t10013
L100154:
add t14700, t10014, #1
movw t163, #4
mul t14800, t14700, t163
add t14900, t12200, t14800
add t15000, t10900, #1
movw t164, #4
mul t15100, t15000, t164
ldr t15300, [t12400, t15100]
str t15300, [t14900]
add t11200, t10014, #1
mov t12600, t102
ldr t11300, [t102]
movw t177, #0
cmp t11200, t177
bge L128
L127:
mov t10015, t10014
L100155:
movw t178, #65535
movt t178, #65535
mov r0, t178
bl exit
L128:
mov t10015, t10014
L100156:
cmp t11200, t11300
bge L127
L129:
mov t10016, t10015
L100157:
add t15400, t11200, #1
movw t165, #4
mul t15500, t15400, t165
ldr t15800, [t101]
str t15800, [t12600, t15500]
mov t10008, t10016
b L100144
L102:
mov t10001, t10000
L100158:
movw t179, #0
mov r0, t179
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
movw t135, #32
mov r0, t135
bl malloc
mov t10000, t0
movw t136, #7
str t136, [t10000]
movw t137, #6
str t137, [t10000, #4]
movw t138, #3
str t138, [t10000, #8]
movw t139, #0
str t139, [t10000, #12]
movw t140, #5
str t140, [t10000, #16]
movw t141, #9
str t141, [t10000, #20]
movw t142, #1
str t142, [t10000, #24]
movw t143, #2
str t143, [t10000, #28]
mov t10200, #0
movw t144, #8
mov r0, t144
bl malloc
mov t10400, t0
adr t145, b1^bubbleSort
str t145, [t10400, #4]
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
L100109:
movw t148, #10
mov r0, t148
bl putch
movw t149, #0
mov r0, t149
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov t10202, t10201
L100110:
mov t11700, t10000
ldr t10700, [t10000]
movw t150, #0
cmp t10202, t150
bge L106
L105:
mov t10203, t10202
L100111:
movw t151, #65535
movt t151, #65535
mov r0, t151
bl exit
L106:
mov t10203, t10202
L100112:
cmp t10203, t10700
bge L105
L107:
mov t10204, t10203
L100113:
add t13200, t10204, #1
movw t146, #4
mul t13300, t13200, t146
ldr t11800, [t11700, t13300]
mov r0, t11800
bl putint
movw t147, #32
mov r0, t147
bl putch
add t10205, t10204, #1
mov t10201, t10205
b L100108

