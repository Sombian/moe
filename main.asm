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
	mov DWORD [rbp - 4], rax
	mov rax, 9
	mov DWORD [rbp - 8], rax
	mov rax, 6
	mov rbx, DWORD [rbp - 4]
	mov rcx, DWORD [rbp - 8]
	add rbx, rcx
	mov rcx, 9
	imul rbx, rcx
	imul rax, rbx
	mov DWORD [rbp - 12], rax
	mov rax, 6
	mov rbx, DWORD [rbp - 12]
	mov rcx, 9
	imul rbx, rcx
	imul rax, rbx
	mov DWORD [rbp - 16], rax
	;-------------;
	mov rsp, rbp  ;
	mov rbp, [rsp];
	add rsp, 0x8  ;
	;-------------;
	ret
