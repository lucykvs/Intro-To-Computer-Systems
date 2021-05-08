.pos 0x1000
start:
  ld $stacktop, r5
  gpc $6, r6
  j main
  halt

main:
  deca r5
  st r6, (r5)
  ld $-128, r0
  add r0, r5
  deca r5
  ld $str1, r0
  st r0, (r5)
  gpc $6, r6
  j print                 # goto print
  inca r5
  ld $0, r0
  mov r5, r1
  ld $256, r2
  sys $0                  # read(fd, buffer, size)
  mov r0, r4              # r4 = r0 = number of bytes read in sys call 
  deca r5
  ld $str2, r0
  st r0, (r5)             # r5 = addr of str2
  gpc $6, r6
  j print                 # goto print
  inca r5                 # inc r5 to addr on stack of name entered by user
  ld $1, r0               # r0 = 1
  mov r5, r1              # r1 = addr of name entered by user
  mov r4, r2              # r2 = r4 = number of bytes reads in "read" sys call (num bytes in name user input)
  sys $1                  # writes user-inputted name following print of str2
  ld $128, r0             # r0 = 128
  add r0, r5              # deallocate stack frame
  ld (r5), r6             # load return address (goes to halt)  
  inca r5                 
  j (r6)

print:
  ld (r5), r0             # r0 = addr of arg (struct str)
  ld 4(r0), r1            # load offset of 4 from struct (_str) into r1
  ld (r0), r2             # load offset of 0 from struct (strlen) into r2
  ld $1, r0               # r0 = 1
  sys $1                  # write(1, _str, strlen)
  j (r6)                  # return 

.pos 0x1800
proof:
  deca r5
  ld $str3, r0
  st r0, (r5)
  gpc $6, r6
  j print
  halt

.pos 0x2000
str1:
  .long 30
  .long _str1
_str1:
  .long 0x57656c63
  .long 0x6f6d6521
  .long 0x20506c65
  .long 0x61736520
  .long 0x656e7465
  .long 0x72206120
  .long 0x6e616d65
  .long 0x3a0a0000

str2:
  .long 11
  .long _str2
_str2:
  .long 0x476f6f64
  .long 0x206c7563
  .long 0x6b2c2000

str3:
  .long 43
  .long _str3
_str3:
  .long 0x54686520
  .long 0x73656372
  .long 0x65742070
  .long 0x68726173
  .long 0x65206973
  .long 0x20227371
  .long 0x7565616d
  .long 0x69736820
  .long 0x6f737369
  .long 0x66726167
  .long 0x65220a00

.pos 0x4000
stack:
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
stacktop:
  .long 0
