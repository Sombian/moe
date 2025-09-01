bits 64
default rel

section .text
global _start
main:
	;-------------;
	sub rsp, 0x8  ;
	mov [rsp], rbp;
	mov rbp, rsp  ;
	;-------------;
	mov rax, 6
	mov DWORD [rbp - 4], eax
	mov rax, 4619454727784602010
	movq xmm0, rax
	movss DWORD [rbp - 8], xmm0
	mov rbx, 6
	mov rcx, DWORD [rbp - 4]
	movss xmm0, DWORD [rbp - 8]
	cvtsi2ss xmm1, rcx
	addss xmm1, xmm0
	mov rcx, 9
	imul xmm1, rcx
	imul rbx, xmm1
	movsd QWORD [rbp - 16], rbx
	mov rbx, 6
	movsd xmm0, QWORD [rbp - 16]
	mov rcx, 9
	cvtsi2sd xmm1, rcx
	mulsd xmm0, xmm1
	cvtsi2sd xmm1, rbx
	mulsd xmm1, xmm0
	movss DWORD [rbp - 20], xmm1
	;-------------;
	mov rsp, rbp  ;
	mov rbp, [rsp];
	add rsp, 0x8  ;
	;-------------;
	ret
