.pos 0x100

    ld $a, r0           # r0 = address of a
    ld $i, r1           # r1 = address of i
    ld $3, r2           # r2 = 3
    ld (r0, r2, 4), r3  # r3 = a[3]
    st r3, (r1)         # i = a[3]
    ld (r1), r2         # r2 = value of i
    ld (r0, r2, 4), r1  # i = a[i] 


    halt

.pos 0x200
# Data area

i:  .long 1             # i
j:  .long 2             # j
a:  .long 3             # a[0]
    .long 4             # a[1]
    .long 5             # a[2]
    .long 6             # a[3]
    .long 7             # a[4]
    .long 8             # a[5]
    .long 9             # a[6]
    .long 10            # a[7]
    .long 11            # a[8]
    .long 12            # a[9]
    .long 13            # a[10]
