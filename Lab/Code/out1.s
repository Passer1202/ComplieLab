.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
 li $v0, 4
 la $a0, _prompt
 syscall
 li $v0, 5
 syscall
 jr $ra

write:
 li $v0, 1
 syscall
 li $v0, 4
 la $a0, _ret
 syscall
 move $v0, $0
 jr $ra

fact:
 move $fp, $sp
 addi $sp, $sp, -4
 lw $t0, 12($fp)
 sw $t0, -4($fp)
 lw $t0, -4($fp)
 li $t1, 1
 beq  $t0, $t1, label0
 j label1
label0:
 lw $t0, -4($fp)
 move $v0, $t0
 jr $ra
 j label2
label1:
 addi $sp, $sp, -4
 lw $t0, -4($fp)
 addi $t0, $t0, -1
 sw $t0, -8($fp)
 addi $sp, $sp, -4
 lw $t0, -8($fp)
 sw $t0, 0($sp)
 addi $sp, $sp, -12
 sw $ra, 0($sp)
 sw $fp, 4($sp)
 sw $sp, 8($sp)
 jal fact
 lw $sp, 8($fp)
 lw $fp, 4($sp)
 lw $ra, 0($sp)
 addi $sp, $sp, 12
 addi $sp, $sp, -4
 sw $v0, -16($fp)
 addi $sp, $sp, -4
 lw $t0, -4($fp)
 lw $t1, -16($fp)
 mul $t0, $t0, $t1
 sw $t0, -20($fp)
 lw $t0, -20($fp)
 move $v0, $t0
 jr $ra
label2:

main:
 move $fp, $sp
 addi $sp, $sp, -4
 addi $sp, $sp, -4
 sw $ra, 0($sp)
 jal read
 lw $ra, 0($sp)
 addi $sp, $sp, 4
 sw $v0, -4($fp)
 addi $sp, $sp, -4
 lw $t0, -4($fp)
 sw $t0, -8($fp)
 lw $t0, -8($fp)
 li $t1, 1
 bgt  $t0, $t1, label3
 j label4
label3:
 addi $sp, $sp, -4
 lw $t0, -8($fp)
 sw $t0, 0($sp)
 addi $sp, $sp, -12
 sw $ra, 0($sp)
 sw $fp, 4($sp)
 sw $sp, 8($sp)
 jal fact
 lw $sp, 8($fp)
 lw $fp, 4($sp)
 lw $ra, 0($sp)
 addi $sp, $sp, 12
 addi $sp, $sp, -4
 sw $v0, -16($fp)
 addi $sp, $sp, -4
 lw $t0, -16($fp)
 sw $t0, -20($fp)
 j label5
label4:
 li $t0, 1
 sw $t0, -20($fp)
label5:
 lw $a0, -20($fp)
 addi $sp, $sp, -4
 sw $ra, 0($sp)
 jal write
 lw $ra, 0($sp)
 addi $sp, $sp, 4
 li $v0, 0
 jr $ra
