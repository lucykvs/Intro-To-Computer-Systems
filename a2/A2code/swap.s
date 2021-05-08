.pos 0x100
                 ld   $array, r0          # r0 = address of array
                 ld   $t, r1              # r1 = address of t
                 ld   (r0), r2            # r2 = array[0]
                 st   r2, (r1)            # t = array[0]
                 ld   $0x01, r3           # r3 = 1
                 ld   (r0, r3, 4), r4     # r4 = array[1]
                 st   r4, (r0)            # array[0] = array[1]
                 ld   (r1), r5            # r5 = t
                 st   r5, (r0, r3, 4)     # array[1] = t
                 halt                     # halt
.pos 0x1000
t:               .long 0xffffffff         # t
.pos 0x2000
array:           .long 0x00000031         # array[0]
                 .long 0x00000015         # array[1]