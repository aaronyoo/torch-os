bits 32


;------------------
; Multiboot Header
;------------------
; For details: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Header-layout

MAGIC equ 0x1BADB002                ; Multiboot Magic
FLAGS equ 0                         ; We don't need any flags right now
CHECKSUM equ -(MAGIC + FLAGS)       ; checksum + magic + flags = 0

align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

;---------------
; Kernel Stack
;---------------
section .bss
align 4
kernel_stack_bottom: equ $          ; Create a 16KB stack for the kernel
	resb 16384 ; 16 KB
kernel_stack_top:

;--------------------
; Kernel Entry Point
;--------------------
section .text
global start
start:
    cld                             ; Clear the direction flag for string operations
    mov esp, kernel_stack_top       ; Set up the stack

    ; TODO: I have seen some kernels use this space to push the
    ; Multiboot magic and the header address. I am not sure exactly
    ; what those values are used for in kmain, but maybe consider this.

    extern kmain
    call kmain                      ; Jump to kmain (never to return)

    cli                             ; We should not return here, but if we do:
    hlt                             ;   clear all interrupts and halt
