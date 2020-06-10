.word 1024 8
.word 1025 6
.word 1026 10
.word 1027 1
.word 1028 0
.word 1029 5
.word 1030 7
.word 1031 2
.word 1032 3
.word 1033 20
.word 1034 20
.word 1035 0
.word 1036 23
.word 1037 7
.word 1038 2
.word 1039 1


#  qsort
		    add $sp, $zero, $imm, 1		             # set $sp = 1
	    	sll $sp, $sp, $imm, 11	               	 # set $sp = 1 << 11 = 2048
            add $s0, $zero, $zero, 1
            sll $s0, $sp, $imm, 10                   # $s0=1024 start ad of array A
            add $a0, $zero, $zero, 0                 # p=0
            add $a1, $zero, $imm, 15                 # r=15
            jal $imm, $zero, $zero, QS_TEST          # call recursive function
            halt $zero, $zero, $zero 0               # return from first rec call, meaning A is sorted.

QS_TEST:    
	        add $sp, $sp, $imm, -4		             # adjust stack for 4 items
	        sw $a0, $sp, $imm, 3		             # save p
	        sw $a1, $sp, $imm, 2		             # save r
	        sw $ra, $sp, $imm, 1		             # save return address
            sw $v0, $sp, $imm, 0                     # save q
            ble $imm, $a1, $a0, QS_EXIT              # if  r <= p: end rec call
            beq $imm,  $zero, $zero, PARTITION       # PARTITION calculates & saves q to $v0

QS_EXIT:    
            add $sp, $sp, $imm, 4	               	 # pop 4 items from stack
            beq $ra, $zero, $zero, 0

PARTITION:        
            lw $s2, $s0, $a0, 0                      # $s2 = A[p] = pivot
            add $t2, $a0, $imm, -1                   # $t2 = i = p-1
            add $v0, $a1, $imm, 1                    # $vo = j = r+1
P_JLOOP:    
            add $v0, $v0, $imm, -1                   # j--
            lw $s3, $s0, $v0, 0                      # $s3 = A[j]
            bgt $imm, $s3, $s2, P_JLOOP              # do P_JLOOP while A[j] > pivot

P_ILOOP:    
            add $t2, $t2, $imm, 1                    # i++
            lw $t1, $s0, $t2, 0                      # $t1 = A[i]
            blt $imm, $t1, $s2, P_ILOOP              # do P_ILOOP while A[i] < pivot
            bge $imm, $t2, $v0, P_EXIT               # exit partition without swapping if i>=j
SWAP:       
            add $t3, $s3, $zero, 0                   # $t3 = A[j] save aside
            sw $t1, $s0, $v0, 0                      # A[j] = A[i]
            sw $t3, $s0, $t2, 0                      # A[i] = A[j]
            beq $imm, $zero, $zero, P_JLOOP          # do loops after the switch
P_EXIT:     
            sw $v0, $sp, $imm, 0                     # save q

            add $a1, $v0, $zero, 0                   # set r=q
            jal $imm, $zero, $zero, QS_TEST          # qsort(p,q)
            lw $v0, $sp, $imm, 0                     # restore q
            lw $a1, $sp, $imm, 1                     # restore $ra
            lw $v0, $sp, $imm, 2                     # restore r
            lw $a0, $sp, $imm, 3                     # restore p
            add $a0, $v0, $imm, 1                    # p = q+1
            jal $imm, $zero, $zero, QS_TEST          # qsort(q+1,r)
            lw $v0, $sp, $imm, 0                     # restore q
            lw $a1, $sp, $imm, 1                     # restore $ra
            lw $v0, $sp, $imm, 2                     # restore r
            lw $a0, $sp, $imm, 3                     # restore p
            beq $imm, $zero, $zero, QS_EXIT          # pop items and return