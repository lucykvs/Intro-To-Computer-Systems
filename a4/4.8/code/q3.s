.pos 0x1000
code:
                 # v0 = s.x[i]
                 ld   $v0, r0            # r0 = address of v0
                 ld   $i, r1             # r1 = address of i
                 ld   (r1), r1           # r1 = value of i
                 ld   $s, r2             # r2 = address of s
                 ld   (r2, r1, 4), r3    # r3 = s.x[i]
                 st   r3, (r0)           # v0 = s.x[i]

				 # v1 = s.y[i];
                 ld   $v1, r0            # r0 = address of v1
				 ld   8(r2), r3          # r3 = address of s.y 
                 ld   (r3, r1, 4), r3    # r3 = value of s.y[i]
                 st   r3, (r0)           # v1 = s.y[i]	 
				 
				 # v2 = s.z->x[i];
                 ld   $v2, r0            # r0 = address of v2
                 ld   12(r2), r3         # r3 = address of s.z
                 ld   (r3, r1, 4), r3    # r3 = value of s.z->x[i]
                 st   r3, (r0)           # v2 = s.z->x[i] 
	
	             # v3 = s.z->z->y[i];
				 ld   $v3, r0            # r0 = address of v3
                 ld   12(r2), r3         # r3 = address of s.z
                 ld   12(r3), r3         # r3 = address of s.z->z
                 ld   8(r3), r3          # r3 = address of s.z->z->y
                 ld   (r3, r1, 4), r3    # r3 = value of s.z->z->y[i]
                 st   r3, (r0)           # v3 = s.z->z->y[i]

                 halt                    # halt

.pos 0x2000
static:
i:               .long 0
v0:              .long 0
v1:              .long 0
v2:              .long 0
v3:              .long 0
s:               .long 2          # x[0]
                 .long 3          # x[1]
                 .long s_y        # s.y
                 .long s_z        # s.z                        

.pos 0x3000
heap:
s_y:             .long 4          #s.y[0]
                 .long 5          #s.y[1]
s_z:             .long 6          #s.z[0] or s.z->x[0] 
                 .long 7          #s.z[1] or s.z->x[1]
                 .long 8          #s.z[2] or s.z->y
                 .long s_z_z      #s.z[3] or s.z->z
s_z_z:           .long 9          #s.z->z[0] or s.z->z->x[0] 
                 .long 10         #s.z->z[1] or s.z->z->x[1]
                 .long s_z_z_y    #s.z->z[2] or s.z->z->y
                 .long 11         #s.z->z[3] or s.z->z->z
s_z_z_y:         .long 12         #s.z->z->y[0]
                 .long 13         #s.z->z->y[1]
                                                   