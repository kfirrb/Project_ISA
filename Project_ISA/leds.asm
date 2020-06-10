			beq $imm, $zero, $zero, SETUP	# branch to SETUP
LIGHT:
			out $zero, $zero, $imm, 3		# clear irq0 status
			add $t0, $t0, $imm, 1			# i++
			bgt $imm, $t0, $t1, END			# check if i>n, if so exit. else light next
			sll $t2, $s1, $t0, 0			# move '1' to the next led spot
			out $t2, $imm, $zero, 9			# light the next led
			reti $zero, $zero, $zero, 0		# return from interrupt
END:
			out $zero, $zero, $imm, 3		# clear irq0 status
			halt $zero, $zero, $zero, 0		# i > 31 so exit program
SETUP:
			add $t0, $zero, $imm, 6			# set $t0 = 6
			out $imm, $t0, $zero, LIGHT		# set irqhandler as LIGHT
			add $s1, $imm, $zero, 1			# $s1 = 1
			out $s1, $zero, $imm, 0			# enable irq0
			add $s0, $imm, $zero, 256
			out $s0, $imm, $zero, 13		# set timermax =256 so that irqstatus is enabled every sec
			add $t1, $imm, $zero, 31		# n = 31
			add $t0, $imm, $zero, -1		# i = -1
			out $s1, $zero, $imm, 11		# enable timer

