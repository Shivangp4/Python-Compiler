	.section	.rodata
.LC0:
	.string	"__main__"
.LC1:
	.string	"%d\n"

	.text

	.globl	A.__init__
	.type	A.__init__, @function
A.__init__:
	# beginfunc
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rdi, -16(%rbp)

	# self.x = 1
	movq	-16(%rbp), %rax
	leaq	0(%rax), %rdx

	movl	$1, %eax
	movl	%eax, (%rdx)

	# self.y = 3
	movq	-16(%rbp), %rax
	leaq	4(%rax), %rdx

	movl	$3, %eax
	movl	%eax, (%rdx)

	# return
	leave
	ret

	# endfunc
	.size	A.__init__, .-A.__init__

	.globl	B.__init__
	.type	B.__init__, @function
B.__init__:
	# beginfunc
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movq	%rdi, -16(%rbp)

	# self.x = 1
	movq	-16(%rbp), %rax
	leaq	0(%rax), %rdx

	movl	$1, %eax
	movl	%eax, (%rdx)

	# self.y = 3
	movq	-16(%rbp), %rax
	leaq	4(%rax), %rdx

	movl	$3, %eax
	movl	%eax, (%rdx)

	# self.z = 5
	movq	-16(%rbp), %rax
	leaq	0(%rax), %rdx

	movl	$5, %eax
	movl	%eax, (%rdx)

	# return
	leave
	ret

	# endfunc
	.size	B.__init__, .-B.__init__

	.globl	main
	.type	main, @function
main:
	# beginfunc
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp

	# t1 = 8
	movl	$8, %eax
	movl	%eax, -4(%rbp)

	# call allocmem , 1
	movl	-4(%rbp), %edi
	call	malloc@PLT

	# t2 = popparam
	movq	%rax, -12(%rbp)

	# call A.__init__ , 1
	movq	-12(%rbp), %rdi
	call	A.__init__

	# a = t2
	movq	-12(%rbp), %rax
	movq	%rax, -20(%rbp)

	# t3 = 12
	movl	$12, %eax
	movl	%eax, -24(%rbp)

	# call allocmem , 1
	movl	-24(%rbp), %edi
	call	malloc@PLT

	# t4 = popparam
	movq	%rax, -32(%rbp)

	# call B.__init__ , 1
	movq	-32(%rbp), %rdi
	call	B.__init__

	# b = t4
	movq	-32(%rbp), %rax
	movq	%rax, -40(%rbp)

	# x = a.x
	movq	-20(%rbp), %rax
	leaq	0(%rax), %rdx

	movl	(%rdx), %eax
	movl	%eax, -44(%rbp)

	# y = a.y
	movq	-20(%rbp), %rax
	leaq	4(%rax), %rdx

	movl	(%rdx), %eax
	movl	%eax, -48(%rbp)

	# call print , 1
	movl	-44(%rbp), %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf@PLT

	# call print , 1
	movl	-48(%rbp), %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf@PLT

	# return
	leave
	ret

	# endfunc
	.size	main, .-main

L1:
	# t1 = __name__ == "__main__"
	movl	-8(%rbp), %eax
	cmpl	$.LC0, %eax
	sete	%al
	movb	%al, -9(%rbp)

	# if t1 goto L2
	movb	-9(%rbp), %al
	cmpb	$0, %al
	jg	L2

	# goto L3
	jmp	L3

L2:
	# call main , 0
	call	main

	# goto L4
	jmp	L4

L3:
L4:
