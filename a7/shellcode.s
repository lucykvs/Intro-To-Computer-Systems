  gpc $10, r0          # r0 = addr of /bin/sh
  ld $7, r1            # r1 = size
  sys $2               # system call: exec
  halt                 # halt

  .long 0x2f62696e     # /bin
  .long 0x2f736800     # /sh
