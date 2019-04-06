.section .init
.global _start
_start:

ldr r0, =0x3F200000
ldr r2, =0x3F003000
ldr r5, =0x1e8480
ldr r6, =0x7a120

loop:
mov r1, #1
lsl r1, #27
str r1, [r0, #8]

mov r1, #1
lsl r1, #29
str r1,[r0,#28]

ldr r3,[r2, #4]
loop2:
ldr r4, [r2, #4]
sub r4,r4,r3
cmp r4,r6
bne loop2

mov r1, #1
lsl r1, #29
str r1, [r0, #40]

ldr r3,[r2, #4]
loop1:
ldr r4, [r2, #4]
sub r4,r4,r3
cmp r4,r5
bne loop1
b loop
