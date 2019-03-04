section .boot
bits 16
global boot
boot:
	mov ax, 0x2401                ; enable A20 gate
	int 0x15

	mov ax, 0x3                   ; set VGA text mode 3
	int 0x10

	mov [disk],dl

	mov ah, 0x2                   ; read sectors
	mov al, 6                     ; sectors to read
	mov ch, 0                     ; cylinder index
	mov dh, 0                     ; head index
	mov cl, 2                     ; sector index
	mov dl, [disk]                ; disk index
	mov bx, boot2                ; target pointer
	int 0x13

	cli                           ; disable interrupts

	lgdt [gdt_pointer]            ; load GDT pointer

	mov eax, cr0                  ; put protectd mode bit in register cr0
	or eax,0x1
	mov cr0, eax

	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp CODE_SEG:boot2

  ;; GDT Table Deinition
gdt_start:
	dq 0x0
gdt_code:
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0
gdt_data:
	dw 0xFFFF
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0
gdt_end:
gdt_pointer:
	dw gdt_end - gdt_start
	dd gdt_start

disk:
	db 0x0
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

times 510 - ($-$$) db 0
dw 0xaa55


bits 32
boot2:
	mov esp,kernel_stack_top
	extern kernel_main
	call kernel_main
	cli
	hlt

section .bss
align 4
kernel_stack_bottom: equ $
	resb 16384 ; 16 KB
kernel_stack_top:
