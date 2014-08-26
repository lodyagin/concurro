	.file	"stack.cpp"
	.section	.text.unlikely._ZNKSt5ctypeIcE8do_widenEc,"axG",@progbits,_ZNKSt5ctypeIcE8do_widenEc,comdat
	.align 2
.LCOLDB0:
	.section	.text._ZNKSt5ctypeIcE8do_widenEc,"axG",@progbits,_ZNKSt5ctypeIcE8do_widenEc,comdat
.LHOTB0:
	.align 2
	.p2align 4,,15
	.weak	_ZNKSt5ctypeIcE8do_widenEc
	.type	_ZNKSt5ctypeIcE8do_widenEc, @function
_ZNKSt5ctypeIcE8do_widenEc:
.LFB1046:
	.cfi_startproc
	movl	%esi, %eax
	ret
	.cfi_endproc
.LFE1046:
	.size	_ZNKSt5ctypeIcE8do_widenEc, .-_ZNKSt5ctypeIcE8do_widenEc
	.section	.text.unlikely._ZNKSt5ctypeIcE8do_widenEc,"axG",@progbits,_ZNKSt5ctypeIcE8do_widenEc,comdat
.LCOLDE0:
	.section	.text._ZNKSt5ctypeIcE8do_widenEc,"axG",@progbits,_ZNKSt5ctypeIcE8do_widenEc,comdat
.LHOTE0:
	.section	.text.unlikely._ZN2hc5stack3topEv,"axG",@progbits,_ZN2hc5stack3topEv,comdat
.LCOLDB1:
	.section	.text._ZN2hc5stack3topEv,"axG",@progbits,_ZN2hc5stack3topEv,comdat
.LHOTB1:
	.p2align 4,,15
	.weak	_ZN2hc5stack3topEv
	.type	_ZN2hc5stack3topEv, @function
_ZN2hc5stack3topEv:
.LFB1306:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	0(%rbp), %rax
	movq	8(%rbp), %rdx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1306:
	.size	_ZN2hc5stack3topEv, .-_ZN2hc5stack3topEv
	.section	.text.unlikely._ZN2hc5stack3topEv,"axG",@progbits,_ZN2hc5stack3topEv,comdat
.LCOLDE1:
	.section	.text._ZN2hc5stack3topEv,"axG",@progbits,_ZN2hc5stack3topEv,comdat
.LHOTE1:
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC2:
	.string	"stack (fp, ip):\n"
.LC3:
	.string	", "
	.section	.text.unlikely,"ax",@progbits
.LCOLDB4:
	.text
.LHOTB4:
	.p2align 4,,15
	.type	_Z3reqi.part.4, @function
_Z3reqi.part.4:
.LFB1478:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movl	$16, %edx
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movl	$.LC2, %esi
	movl	$_ZSt4cout, %edi
	subq	$24, %rsp
	.cfi_def_cfa_offset 64
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	call	_ZN2hc5stack3topEv
	movq	%rax, %rbx
	movq	%rdx, %r12
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L25:
	movsbl	67(%rbp), %esi
.L7:
	movq	%r12, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	testb	%r13b, %r13b
	jne	.L14
	movq	8(%rbx), %r12
	movq	(%rbx), %rbx
.L5:
	testq	%rbx, %rbx
	sete	%r13b
	jne	.L15
	testq	%r12, %r12
	je	.L4
.L15:
	leaq	15(%rsp), %rsi
	movl	$1, %edx
	movl	$_ZSt4cout, %edi
	movb	$40, 15(%rsp)
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%rbx, %rsi
	movq	%rax, %rdi
	call	_ZNSo9_M_insertIPKvEERSoT_
	movl	$2, %edx
	movq	%rax, %rbp
	movl	$.LC3, %esi
	movq	%rax, %rdi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%r12, %rsi
	movq	%rbp, %rdi
	call	_ZNSo9_M_insertIPKvEERSoT_
	leaq	14(%rsp), %rsi
	movl	$1, %edx
	movq	%rax, %rdi
	movb	$41, 14(%rsp)
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%rax, %r12
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%r12,%rax), %rbp
	testq	%rbp, %rbp
	je	.L24
	cmpb	$0, 56(%rbp)
	jne	.L25
	movq	%rbp, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	0(%rbp), %rax
	movl	$10, %esi
	movq	48(%rax), %rax
	cmpq	$_ZNKSt5ctypeIcE8do_widenEc, %rax
	je	.L7
	movq	%rbp, %rdi
	call	*%rax
	movsbl	%al, %esi
	jmp	.L7
	.p2align 4,,10
	.p2align 3
.L14:
	xorl	%r12d, %r12d
	xorl	%ebx, %ebx
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L4:
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L24:
	.cfi_restore_state
	call	_ZSt16__throw_bad_castv
	.cfi_endproc
.LFE1478:
	.size	_Z3reqi.part.4, .-_Z3reqi.part.4
	.section	.text.unlikely
.LCOLDE4:
	.text
.LHOTE4:
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	_Z3reqi
	.type	_Z3reqi, @function
_Z3reqi:
.LFB1312:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	leal	(%rdi,%rdi), %ebp
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	movl	%edi, %ebx
	movl	$_ZSt4cout, %edi
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movl	%ebp, %esi
	call	_ZNSolsEi
	testl	%ebx, %ebx
	jle	.L28
.L27:
	leal	-2(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	cmpl	$1, %ebx
	je	.L28
	leal	-4(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	cmpl	$2, %ebx
	je	.L28
	leal	-6(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	leal	-3(%rbx), %eax
	testl	%eax, %eax
	jle	.L28
	leal	-8(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	leal	-4(%rbx), %eax
	testl	%eax, %eax
	jle	.L28
	leal	-10(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	leal	-5(%rbx), %eax
	testl	%eax, %eax
	jle	.L28
	leal	-12(%rbp), %esi
	movl	$_ZSt4cout, %edi
	subl	$14, %ebp
	call	_ZNSolsEi
	leal	-6(%rbx), %eax
	testl	%eax, %eax
	jle	.L28
	subl	$7, %ebx
	movl	%ebp, %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	testl	%ebx, %ebx
	jg	.L27
	.p2align 4,,10
	.p2align 3
.L28:
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	jmp	_Z3reqi.part.4
	.cfi_endproc
.LFE1312:
	.size	_Z3reqi, .-_Z3reqi
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.text.unlikely
.LCOLDB6:
	.text
.LHOTB6:
	.p2align 4,,15
	.globl	_Z1fi
	.type	_Z1fi, @function
_Z1fi:
.LFB1313:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	leal	(%rdi,%rdi), %ebp
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	movl	%edi, %ebx
	movl	$_ZSt4cout, %edi
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movl	%ebp, %esi
	call	_ZNSolsEi
	testl	%ebx, %ebx
	jle	.L39
	leal	-2(%rbp), %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSolsEi
	cmpl	$1, %ebx
	je	.L39
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	leal	-2(%rbx), %edi
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	jmp	_Z3reqi
	.p2align 4,,10
	.p2align 3
.L39:
	.cfi_restore_state
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	jmp	_Z3reqi.part.4
	.cfi_endproc
.LFE1313:
	.size	_Z1fi, .-_Z1fi
	.section	.text.unlikely
.LCOLDE6:
	.text
.LHOTE6:
	.section	.rodata.str1.1
.LC7:
	.string	"\n\n"
	.section	.text.unlikely
.LCOLDB8:
	.section	.text.startup,"ax",@progbits
.LHOTB8:
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB1314:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movq	_ZSt4cout(%rip), %rax
	movq	__libc_stack_end(%rip), %rsi
	movl	$_ZSt4cout, %edi
	movq	-24(%rax), %rdx
	movl	_ZSt4cout+24(%rdx), %eax
	andl	$-75, %eax
	orl	$8, %eax
	movl	%eax, _ZSt4cout+24(%rdx)
	call	_ZNSo9_M_insertIPKvEERSoT_
	movl	$.LC7, %esi
	movq	%rax, %rdi
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
	movl	$5, %edi
	call	_Z3reqi
	xorl	%eax, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1314:
	.size	main, .-main
	.section	.text.unlikely
.LCOLDE8:
	.section	.text.startup
.LHOTE8:
	.section	.text.unlikely
.LCOLDB9:
	.section	.text.startup
.LHOTB9:
	.p2align 4,,15
	.type	_GLOBAL__sub_I__Z3reqi, @function
_GLOBAL__sub_I__Z3reqi:
.LFB1473:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$_ZStL8__ioinit, %edi
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, %edx
	movl	$_ZStL8__ioinit, %esi
	movl	$_ZNSt8ios_base4InitD1Ev, %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	__cxa_atexit
	.cfi_endproc
.LFE1473:
	.size	_GLOBAL__sub_I__Z3reqi, .-_GLOBAL__sub_I__Z3reqi
	.section	.text.unlikely
.LCOLDE9:
	.section	.text.startup
.LHOTE9:
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I__Z3reqi
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.hidden	__dso_handle
	.ident	"GCC: (Debian 4.9.1-4) 4.9.1"
	.section	.note.GNU-stack,"",@progbits
