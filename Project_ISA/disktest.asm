# disktest
		add $v0, $zero, $zero, 0			# i = 0 sector number
		add $s0, $imm, $zero, 1024			# 1024 is base address of sector 0 in memory
		add $s2, $imm,$zero, 1			
		add $sp, $imm, $zero, 3
		add $t0, $zero, $imm, 6				# set $t0 = 6
		out $imm, $t0, $zero, DISK			# set irqhandler as DISK

READ:
		bgt $imm, $v0, $sp, END				# if i>3 we copied all 4 sectors
		sll $t0, $v0, $imm, 7
		add $t0, $t0, $s0, 0				
		add $a0, $t0, $zero, 0				# $a0 = i*128 + 1024 base ad of current sector
		add $a1, $v0, $zero, 0				# $a1 = sector number
		add $s1, $imm, $zero, 1				# $s1 = 1 read cmd
		beq $imm, $zero, $zero, HOLD
WRITE:
		add $t0, $v0, $imm, 4				# t0 = i+4
		add $a1, $t0, $zero, 0				# sector num is i+4
		add $s1, $imm, $zero, 2				# s1 set to write cmd
		beq $imm, $zero, $zero, HOLD

DISK:
		in $t2, $zero, $imm, 17				# get diskstatus value
		bne $imm, $t2, $zero, READ			# redo the same iteration if disk isn't free
		out $a0, $imm, $zero, 16			# set sector buffer ad
		out $a1, $imm, $zero, 15			# set sector number
		out $s1, $imm, $zero, 14			# set r/w
		out $zero, $zero, $imm, 1			# clear irq1
		beq $imm, $s1, $s2, WRITE			# branch to WRITE if last iteration was a read
		add $v0, $imm, $v0, 1				# i++
		beq $imm, $zero, $zero, READ		# else, do READ for next sector
END:
		halt $zero, $zero, $zero, 0			# exit program
HOLD:
		out $imm, $zero, $imm, 1			# enable irq1
		 
