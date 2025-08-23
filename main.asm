bits 64
default rel

section .text
global _start
	sub rsp, 4
	mov DWORD PTR [rbp - 4], 6
	sub rsp, 4
	mov DWORD PTR [rbp - 8], 9
main:
	sub rsp, 0x8
	mov [rsp], rbp
	mov rbp, rsp
	sub rsp, 4
	mov DWORD PTR [rbp - 4], 6
	sub rsp, 4
	mov DWORD PTR [rbp - 8], 9
	sub rsp, 4
	mov DWORD PTR [rbp - 12], %s
	mov rsp, rbp
	mov rbp, [rsp]
	add rsp, 0x8
	ret
