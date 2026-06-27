.global C^m
C^m:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #100
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t111, #4
mov r0, t111
bl malloc
mov t10200, t0
adr t112, C^m
str t112, [t10200]
mov t10001, t10200
ldr t10400, [t10001]
mov r12, t10400
mov r0, t10001
blx r12
mov t10500, t0
mov r0, t10500
bl putint
movw t113, #10
mov r0, t113
bl putch
movw t114, #0
mov r0, t114
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

