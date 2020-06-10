# clock
			beq $imm, $zero, $zero, SETUP		# branch to SETUP
TEST:
			beq $imm, $t0, $t1, EIGHT			# check if it's 00195959, if so jump to EIGHT
			beq $imm, $zero, $zero, INC
INC:
			add $t0, $t0, $imm, 1
			out $t0, $zero, $imm, 10			# display new time
			out $zero, $zero, $imm, 3			# clear irq0 status
			beq $imm, $t0, $t3, END				# if the time is 00200005: END
			reti $zero, $zero, $zero, 0			# return from interrupt

EIGHT:		
			add $t0, $t2, $zero, 0				# t0 is now 00200000
			out $t0, $zero, $imm, 10			# display new time
			out $zero, $zero, $imm, 3			# clear irq0 status
			reti $zero, $zero, $zero, 0			# return from interrupt
END:
			halt $zero, $zero, $zero, 0

SETUP:
			add $t0, $zero, $imm, 6				# set $t0 = 6
			out $imm, $t0, $zero, TEST			# set irqhandler as TEST
			add $sp, $imm, $zero, 1				# $sp = 1
			out $sp, $zero, $imm, 0				# enable irq0
			add $s0, $zero, $imm, 256
			out $s0, $imm, $zero, 13			# set timermax = 256 so that irqstatus is enabled every sec
			out $zero, $imm, $zero, 10			# display 00000000
			in $t0, $imm, $zero, 10
			or $t0, $t0, $imm, 1
			sll $t0, $t0, $imm, 4
			or $t0, $t0, $imm, 9
			sll $t0, $t0, $imm, 4
			or $t0, $t0, $imm, 5
			sll $t0, $t0, $imm, 4
			or $t0, $t0, $imm, 9
			sll $t0, $t0, $imm, 4
			or $t0, $t0, $imm, 5
			sll $t0, $t0, $imm, 4
			or $t0, $t0, $imm, 5	
			out $t0, $zero, $imm, 10			# displaying $t0 = 00195955
			add $t1, $t0, $imm, 4				# $t1 = 00195959
			add $t2, $imm, $zero, 2	
			sll $t2, $t2, $imm, 20				# $t2 = 00200000
			add $t3, $t2, $imm, 5				# $t3 = 00200005
			out $sp, $zero, $imm, 11			# enable timer