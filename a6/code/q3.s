## Code
.pos 0x1000

# iterate through list of students, compute average grade for each and store them
    ld $s, r0           # r0 = &s
    ld (r0), r0         # r0 = s = base
    ld $n, r1           # r1 = &n
    ld (r1), r1         # r1 = n = n'
    not r1              # r1 = ~n'
    inc r1              # r1 = -n'
    ld $0, r2           # r2 = i' = 0

L0: mov r2, r3          # r3 = i'
    add r1, r3          # r3 = i' - n'
    beq r3, L2          # goto L2 if i' == n' (all averages have been computed)
    br L1               # goto L1

L1: mov r2, r3          # r3 = i'
    mov r2, r4          # r4 = i'
    shl $4, r3          # r3 = i' * 16
    shl $3, r4          # r4 = i' * 8
    add r4, r3          # r3 = i' * 24
    add r0, r3          # r3 = base + i' * 24 = &s[i]     
    ld 4(r3), r4        # r4 = s[i].grade[0]
    ld 8(r3), r5        # r5 = s[i].grade[1]
    add r5, r4          # r4 = s[i].grade[0] + s[i].grade[1]
    ld 12(r3), r5       # r5 = s[i].grade[2]
    add r5, r4          # r4 = s[i].grade[0] + s[i].grade[1] + s[i].grade[2]
    ld 16(r3), r5       # r5 = s[i].grade[3]
    add r5, r4          # r4 = s[i].grade[0] + s[i].grade[1] + s[i].grade[2] + s[i].grade[3]
    shr $2, r4          # r4 = s[i].average
    st r4, 20(r3)       # store average in memory
    inc r2              # r2 = i'++
    br L0               # goto top of loop


# use bubblesort to sort students in order of ascending average
L2: ld $s, r0           # r0 = &s
    ld (r0), r0         # r0 = s = base
    ld $n, r1           # r1 = &n
    ld (r1), r1         # r1 = n = n'
    not r1              # r1 = ~n'
    inc r1              # r1 = -n'
    ld $0, r2           # r2 = i' = 0

L3: mov r2, r3          # r3 - i'                <- top of outer for loop (i')
    add r1, r3          # r3 = i' - n' 
    beq r3, L7          # goto L7 if i' == n'
    mov r2, r3          # r3 = i'
    inc r3              # r3 = j' = i' + 1

L4: mov r3, r4          # r4 = j'                <- top of inner for loop (j')
    add r1, r4          # r4 = j' - n'
    beq r4, L6          # goto L6 if j' == n'

    mov r2, r4          # r4 = i'
    mov r2, r5          # r5 = i'
    shl $4, r4          # r4 = i' * 16
    shl $3, r5          # r5 = i' * 8
    add r5, r4          # r4 = i' * 24
    add r0, r4          # r4 = base + i' * 24 = &s[i] 
    
    mov r3, r5          # r5 = j'
    mov r3, r6          # r6 = j'
    shl $4, r5          # r5 = j' * 16
    shl $3, r6          # r6 = j' * 8
    add r6, r5          # r5 = j' * 24
    add r0, r5          # r5 = base + j' * 24 = &s[j]

    ld 20(r4), r6       # r6 = s[i].average
    ld 20(r5), r7       # r7 = s[j].average
    not r6              # r6 = ~s[i].average
    inc r6              # r6 = -s[i].average
    add r7, r6          # r6 = s[j].average - s[i].average
    bgt r6, L5          # goto L5 if s[j].average > s[i].average    <- if statement
    beq r6, L5          # goto L5 if s[j].average == s[i].average   <- if statement

# swap s[j] and s[i] if necessary (use r6, r7 to store values being overwritten, store alternating values)
    ld 0(r4), r6        # r6 = s[i].sid
    ld 0(r5), r7        # r7 = s[j].sid
    st r6, 0(r5)        # s[j].sid = s[i].sid
    st r7, 0(r4)        # s[i].sid = s[j].sid
    ld 4(r4), r6        # r6 = s[i].grade[0]
    ld 4(r5), r7        # r7 = s[j].grade[0]
    st r6, 4(r5)        # s[j].sid = s[i].grade[0]
    st r7, 4(r4)        # s[i].sid = s[j].grade[0]
    ld 8(r4), r6        # r6 = s[i].grade[1]
    ld 8(r5), r7        # r7 = s[j].grade[1]
    st r6, 8(r5)        # s[j].sid = s[i].grade[1]
    st r7, 8(r4)        # s[i].sid = s[j].grade[1]
    ld 12(r4), r6        # r6 = s[i].grade[2]
    ld 12(r5), r7        # r7 = s[j].grade[2]
    st r6, 12(r5)        # s[j].sid = s[i].grade[2]
    st r7, 12(r4)        # s[i].sid = s[j].grade[2]
    ld 16(r4), r6        # r6 = s[i].grade[3]
    ld 16(r5), r7        # r7 = s[j].grade[3]
    st r6, 16(r5)        # s[j].sid = s[i].grade[3]
    st r7, 16(r4)        # s[i].sid = s[j].grade[3]
    ld 20(r4), r6        # r6 = s[i].average
    ld 20(r5), r7        # r7 = s[j].average
    st r6, 20(r5)        # s[j].sid = s[i].average
    st r7, 20(r4)        # s[i].sid = s[j].average

L5: inc r3              # r3 = j'++
    br L4               # goto top of inner loop

L6: inc r2              # i' = i'++
    br L3               # goto top of outer loop


# find median average grade of list, store that student's sid in m
# assume n is odd, e.g. median will be at index = (n/2) + 1 (w integer division)
L7: ld $s, r0           # r0 = &s
    ld (r0), r0         # r0 = s = base
    ld $n, r1           # r1 = &n
    ld (r1), r1         # r1 = n
    shr $1, r1          # r1 = n/2
    mov r1, r2          # r2 = i
    mov r1, r3          # r3 = i
    shl $4, r2          # r2 = i * 16
    shl $3, r3          # r3 = i * 8
    add r3, r2          # r2 = i * 24
    add r0, r2          # r2 = base + i' * 24 = &s[i]
    ld 0(r2), r3        # r3 = sid of student w median average
    ld $m, r4           # r4 = &m
    st r3, (r4)         # m = sid of student w median average
    halt                # halt


## Globals
.pos 0x2000   
n:      .long 5       # three students
m:      .long 0       # put the answer here
s:      .long base    # address of the array


## Heap (labels represent dynamic values, not available to code)
.pos 0x3000
base:   .long 1110    # student ID 0        addr = base = base + i*24 + 0
        .long 90      # grade 0             addr = base + 4 = base + i*24 + 4
        .long 96      # grade 1             addr = base + 8 = base + i*24 + 8 
        .long 88      # grade 2             addr = base + 12 = base + i*24 + 12
        .long 90      # grade 3             addr = base + 16 = base + i*24 + 16
        .long 0       # computed average 0  addr = base + 20 = base + i*24 + 20
        .long 1111    # student ID 1        addr = base + 24 = base + i*24 + 0
        .long 90      # grade 0             addr = base + 28 = base + i*24 + 4
        .long 86      # grade 1             addr = base + 32 = base + i*24 + 8
        .long 65      # grade 2             addr = base + 36 = base + i*24 + 12
        .long 70      # grade 3             addr = base + 40 = base + i*24 + 16
        .long 0       # computed average 1  addr = base + 44 = base + i*24 + 20
        .long 1112    # student ID 2        addr = base + 48 = base + i*24 + 0
        .long 90      # grade 0             addr = base + 52 = base + i*24 + 4
        .long 97      # grade 1             addr = base + 56 = base + i*24 + 8
        .long 97      # grade 2             addr = base + 60 = base + i*24 + 12
        .long 97      # grade 3             addr = base + 64 = base + i*24 + 16
        .long 0       # computed average 2  addr = base + 68 = base + i*24 + 20
        .long 1113    # student ID 2        addr = base + 48 = base + i*24 + 0
        .long 42      # grade 0             addr = base + 52 = base + i*24 + 4
        .long 42      # grade 1             addr = base + 56 = base + i*24 + 8
        .long 36      # grade 2             addr = base + 60 = base + i*24 + 12
        .long 64      # grade 3             addr = base + 64 = base + i*24 + 16
        .long 0       # computed average 3  addr = base + 68 = base + i*24 + 20  
        .long 1114    # student ID 2        addr = base + 48 = base + i*24 + 0
        .long 60      # grade 0             addr = base + 52 = base + i*24 + 4
        .long 50      # grade 1             addr = base + 56 = base + i*24 + 8
        .long 80      # grade 2             addr = base + 60 = base + i*24 + 12
        .long 60      # grade 3             addr = base + 64 = base + i*24 + 16
        .long 0       # computed average 4  addr = base + 68 = base + i*24 + 20 