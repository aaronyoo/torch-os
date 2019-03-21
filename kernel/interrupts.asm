;; Macro for ISR that has no error code
%macro ISR_NOERRCODE 1
    global isr%1        
    isr%1:
        cli                     ; disable interrupts
        push byte 0             ; push dummy error code for stack consistency
        push byte %1            ; push interrupt number
        jmp isr_common_stub     ; jump to common stub
%endmacro

;; Macro for ISR that pushes an error code
;; Intel manual says that only interrupts 8, 10-14 push error codes
;; This means that we do not need to push a dummy error code
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli
        push byte %1
        jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

extern isr_handler
isr_common_stub:
    pusha

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10  ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop eax        ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    sti
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP








; %macro IRQ 2
;   global irq%1
;   irq%1:
;     cli
;     push byte 0
;     push byte %2
;     jmp irq_common_stub
; %endmacro

; ; Remapping the hardware interrupts??
; IRQ  0,  32
; IRQ  1,  33
; IRQ  2,  34
; IRQ  3,  35
; IRQ  4,  36
; IRQ  5,  37
; IRQ  6,  38
; IRQ  7,  39
; IRQ  8,  40
; IRQ  9,  41
; IRQ  10,  42
; IRQ  11,  43
; IRQ  12,  44
; IRQ  13,  45
; IRQ  14,  46
; IRQ  15,  47