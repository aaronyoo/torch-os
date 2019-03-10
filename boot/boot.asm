;=====================
; Stage 1 Bootloader
;=====================
section .boot
bits 16
global boot
boot:
	mov ax, 0x2401                ; enable A20 gate
	int 0x15

	mov ax, 0x3                   ; set VGA text mode 3
	int 0x10

	mov [disk],dl				  ; disk is stored in dl

	mov ah, 0x2                   ; read sectors
	mov al, 6                     ; sectors to read
	mov ch, 0                     ; cylinder index
	mov dh, 0                     ; head index
	mov cl, 2                     ; sector index
	mov dl, [disk]                ; disk index
	mov bx, boot2                 ; target pointer
	int 0x13

	cli                           ; disable interrupts
	lgdt [gdt_pointer]            ; load GDT pointer

	mov eax, cr0                  ; put protectd mode bit in register cr0
	or eax,0x1
	mov cr0, eax

	mov ax, DATA_SEG			  ; reset all data segments
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp CODE_SEG:boot2			  ; far jump to fix code segment

;-----------------------
; GDT Table Definition
;-----------------------
gdt_start:
	; null descriptor
	dq 0x0

gdt_code:
	; code descriptor
	dw 0xFFFF 				; limit low
	dw 0x0 					; base low
	db 0x0 					; base middle
	db 10011010b 			; access
	db 11001111b 			; granularity
	db 0x0 					; base high
gdt_data:
	; data descriptor
	dw 0xFFFF 				; limit low
	dw 0x0 					; base low
	db 0x0 					; base middle
	db 10010010b 			; access
	db 11001111b 			; granularity
	db 0x0 					; base high
gdt_end:

gdt_pointer:
	dw gdt_end - gdt_start - 1	; size of GDT
	dd gdt_start				; base of GDT

;------------
; Variables
;------------
disk:
	db 0x0

CODE_SEG equ gdt_code - gdt_start	; code segment offset
DATA_SEG equ gdt_data - gdt_start	; data segment offset

times 510 - ($-$$) db 0				; fill to 512 byte boot sector
dw 0xaa55							; boot sector magic




bits 32
;=====================
; Stage 2 ENTRY POINT
;=====================
boot2:
	mov esp,kernel_stack_top
	extern kmain
	call kmain
	cli
	hlt

section .bss
align 4
kernel_stack_bottom: equ $
	resb 16384 ; 16 KB
kernel_stack_top:
