.pos 0x100
                 ld   $i, r0              # r0 = address of i
                 ld   $x, r1              # r1 = address of x
                 ld   $y, r2              # r2 = address of y
                 ld   $data, r3           # r3 = address of data
                 ld   (r0), r4            # r4 = i
                 ld   (r3, r4, 4), r5     # r5 = data[i]
                 ld   $1, r6              # r6 = 1
                 add  r4, r6              # r6 = i + 1
                 ld   (r3, r6, 4), r6     # r6 = data[i + 1]
                 add  r5, r6              # r6 = data[i] + data[i + 1]
                 st   r6, (r2)            # y = r6 (y = data[i] + data[i + 1])
                 ld   $0xff, r7           # r7 = 0xff
                 and  r7, r6              # r6 = r6 & r7 (y & 0xff) 
                 st   r6, (r1)            # x = r6 (x = y & 0xff)
                 halt                     # halt

.pos 0x1000
i:               .long 0x00000002         # i
.pos 0x1004
x:               .long 0x00000012         # x
.pos 0x1008
y:               .long 0x00000012         # y
.pos 0x2000
data:            .long 0x00000005         # data[0]
                 .long 0x00000004         # data[1]
                 .long 0x00000003         # data[2]
                 .long 0x00000002         # data[3]