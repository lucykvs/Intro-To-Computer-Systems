.pos 0x100
start:
    ld   $sb, r5         # r5 = &sb (stack pointer)
    inca r5              # r5 = &sb + 4; free start's stack
    gpc  $6, r6          # r6 = pc; set return address for main
    j    main            # goto main
    halt

f:
    deca r5              # r5 = r5 - 4; allocate size of f's frame
    ld   $0, r0          # r0 = j = 0; return value of f
    ld   4(r5), r1       # r1 = x[i-1] = z; load argument from main from stack 
    ld   $0x80000000, r2 # r2 = n = 0x80000000
f_loop:
    beq  r1, f_end       # if z == 0 goto f_end; start of while(z !=0)
    mov  r1, r3          # r3 = z
    and  r2, r3          # r3 = z & n
    beq  r3, f_if1       # if z & n == 0 goto f_if1
    inc  r0              # if z & n != 0, j++
f_if1:
    shl  $1, r1          # r1 = r1*2; z = z*2
    br   f_loop          # goto f_loop; return to top of while loop
f_end:
    inca r5              # r5++; free f's frame
    j    (r6)            # goto return address (to main)

main:
    deca r5              # allocate stack space for return address
    deca r5              #      and size of f's argument
    st   r6, 4(r5)       # save return address on stack
    ld   $8, r4          # r4 = i = 8; init variable for for loop
main_loop:
    beq  r4, main_end    # if i == 0 goto main_end; for loop condition
    dec  r4              # r4 = i - 1
    ld   $x, r0          # r0 = &x
    ld   (r0, r4, 4), r0 # r0 = x[i - 1]
    deca r5              # r5 = r5 - 4; allocate space for f's argument, x[i-1]
    st   r0, (r5)        # store x[i-1] on stack
    gpc  $6, r6          # r6 = pc; set return address for f
    j    f               # goto f
    inca r5              # r5 = r5 + 4; free f's argument
    ld   $y, r1          # r1 = &y
    st   r0, (r1, r4, 4) # y[i - 1] = j = f(x[i-1])
    br   main_loop       # goto main_loop; return to top of for loop 
main_end:
    ld   4(r5), r6       # load return address for main into r6
    inca r5              # free space for f's argument
    inca r5              # free main's stack
    j    (r6)            # return 

.pos 0x2000
x:
    .long 1
    .long 2
    .long 3
    .long -1
    .long -2
    .long 0
    .long 184
    .long 340057058

y:
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

.pos 0x8000
# These are here so you can see (some of) the stack contents.
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
sb: .long 0

