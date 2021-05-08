.pos 0x100
                 ld   $a, r0              # r0 = address of a
                 ld   $b, r1              # r1 = address of b
                 ld   (r1), r2            # r2 = b
                 ld   $1, r3              # r3 = 1
                 add  r2, r3              # r3 = b + 1
                 ld   $4, r4              # r4 = 4
                 add  r4, r3              # r3 = r3 + 4
                 shr  $1, r3              # r3 = r3 >> 1 or r3 = r3 / 2
                 and  r2, r3              # r3 = r3 & b
                 shl  $2, r3              # r3 = r3 << 2 (r3 = result)
                 st   r3, (r0)            # a = r3 (a = result)
                 halt                     # halt

.pos 0x1000
a:               .long 0x00000012         # a 
.pos 0x2000
b:               .long 0x00000005         # b