#<---------------------- Program ----------------------->
.text

main: 
	addi $sp, $sp, -4
	sw $ra, 0($sp)

	jal read
	move $a0, $v0		# move head v0 from read to a0 as argument

	jal sort
	jal print

	lw $ra, 0($sp)
	addi $sp, 4
	jr $ra

# <-------------------- Reading File ---------------------->
read:
	addi $sp, -4
	sw $ra, 0($sp)
	li $t5, 0 			# head of the list = t5

readLoop:
	li $a0, 72			# 64 name + 4 ppd + 4 next
	li $v0, 9
	syscall				# allocate memory

	move $t3, $v0		# t3 = dynamic memory allocator 

	li $v0, 4
	la $a0, promptName
	syscall			    # get user input for name

	move $a0, $t3		# allocate memory to argument vaiable

	li $v0, 8
	li $a1, 64			# max size 64
	syscall				# read user input

lookForLine:					# finding newline
	lb $t1, 0($a0)
	beq $t1, 10, foundLine		# 10 for ascii character of \n
	addi $a0, 1			# search until \n
	j lookForLine

foundLine: 
	li $t1, 32			# 32 for ascii character of space
	sb $t1, 0($a0)		# change \n to space

	move $a0, $t3
	la $a1, done 		# compare argument with DONE
	jal strcmp			# check if it is the end of file 

	beq $v0, $zero, end # system read to done
	bne $t5, $zero, notNull # head is not null
	move $t5, $t3

	j continue

notNull:
	sw $t3, 68($t4)		# next value 

continue: 
	li $v0, 4
	la $a0, promptDiameter
	syscall			    # get user input for diameter

	move $t4, $t3		# t4 runner to store ppd values
	li $v0, 6
	syscall				# read diameter

	mov.s $f1, $f0		# store in f1
	mul.s $f1, $f1, $f1 # d^2

	l.s $f3, quarter
	mul.s $f1, $f1, $f3# 1/4 * d^2

	l.s $f4, PI
	mul.s $f1, $f1, $f4  # 1/4 * d^2 * pi

	li $v0, 4
	la $a0, promptPrice
	syscall			    # get user input for price

	li $v0, 6
	syscall				# read price

	mtc1 $zero, $f2		# check if price is 0
	c.eq.s $f0, $f2		# if equal true, else false
	bc1f notZero
	mtc1 $zero, $f1 	# input 0 for ppd
	j ppd

notZero:
	div.s $f1, $f1, $f0 # 1/4 * d^2 * pi / price

ppd:
	swc1 $f1, 64($t4)	# store in the ppd of node
	j readLoop

end:
	move $v0, $t5		# put the head back into return value
	lw $ra, 0($sp)
	addi $sp, 4
	jr $ra

# <----------------- Function for comparing strings -------------->
strcmp:
	addi $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $a1, 8($sp)

strcmpLoop:
	lb $t1, 0($a0)		# read the first character of arg1 to t1
	lb $t2, 0($a1)		# read the first character of arg2 to t2

	add $t0, $t1, $t2	# run until both t1 and t2 is null
	beq $t0, $zero, equal
	blt $t1, $t2, right
	blt $t2, $t1, left

	addi $a0, 1			# compare next character of argg1
	addi $a1, 1			# compare next character of arg2

	j strcmpLoop

equal: 					# returns 0 if two strings are equal
	li $v0, 0
	j finish

right:
	li $v0, 1			# return 1 if right is greater
	j finish

left:
	li $v0, -1			# return -1 if left is greater
	j finish

finish:
	lw $ra, 0($sp)
	lw $a0, 4($sp)
	lw $a1, 8($sp)
	addi $sp, 12
	jr $ra


# <------------------- Print ------------------->
print:
	addi $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $s0, 8($sp)
	move $s0, $a0		# run with s0

printLoop:
	beq $s0, $zero, printDone
	move $a0, $s0
	li $v0, 4			
	syscall				# print name
	
	lwc1 $f12, 64($s0)	
	li $v0, 2
	syscall				# print ppd

	la $a0, newLine
	li $v0, 4
	syscall				# print \n
	lw $s0, 68($s0)		# current = current->next

	j printLoop

printDone:
	lw $ra, 0($sp)
	lw $a0, 4($sp)
	lw $s0, 8($sp)
	addi $sp, 12
	jr $ra

# <------------------- Sort list ------------------->

sort: 
	addi $sp, -24
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $s0, 8($sp)
	sw $s1, 12($sp)
	sw $s2, 16($sp)
	sw $s3, 20($sp)

	move $s0, $a0

loop1:
	beq $s0, $zero, loop1End	# s0 = i
	move $s1, $s0				# s1 = j
	move $s2, $s0				# s2 = max

loop2:
	beq $s1, $0, loop2End
	move $a0, $s2
	move $a1, $s1
	jal cmpnode

	bne $v0, $zero, skip		# Don't swap if left > right
	move $s2, $s1 				# new max

skip:
	lw $s1, 68($s1) 			# s1 = s1->next
	j loop2

loop2End:
	li $t1, 0
	li $t2, 68					# counter
	move $s3, $s0

swap:
	beq $t1, $t2, stop 

	lw $t3, 0($s2)
	lw $t4, 0($s3)
	sw $t3, 0($s3)
	sw $t4, 0($s2)				# swap values 

	addi $s2, 4 
	addi $s3, 4  
	addi $t1, 4					# increase counter
	j swap

stop:
	lw $s0, 68($s0)				# i = i -> next
	j loop1

loop1End:
	lw $ra, 0($sp)
	lw $a0, 4($sp)
	lw $s0, 8($sp)
	lw $s1, 12($sp)
	lw $s2, 16($sp)
	lw $s3, 20($sp)
	addi $sp, 24
	jr $ra

cmpnode:
	addi $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $a1, 8($sp)

	lwc1 $f0, 64($a0)			# look at ppd of arg1
	lwc1 $f1, 64($a1)			# look at ppd of arg2

	c.lt.s $f0, $f1 			# if left is smaller, return true
	bc1t return0
	c.lt.s $f1, $f0				
	bc1t return1

	jal strcmp 					# if equal, do strcmp for alphabetical order
	bgtz $v0, return1
	j return0

return1:
	li $v0, 1
	j return

return0: 
	li $v0, 0
	j return

return:
	lw $ra, 0($sp)
	lw $a0, 4($sp)
	lw $a1, 8($sp)
	addi $sp, 12
	jr $ra

#<---------------------- Global Data -------------------->
.data
	newLine: .asciiz "\n"
	promptName: .asciiz "Enter n:"
	promptDiameter: .asciiz "Enter d:"
	promptPrice: .asciiz "Enter p:"
	PI: .float 3.14159265358979323846 
	quarter: .float 0.25
	done: .asciiz "DONE "

.globl main
