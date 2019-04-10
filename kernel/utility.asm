;; Contains asm stubs to help cpu context setup

global gdt_flush
gdt_flush:
   mov eax, [esp+4]  ; eax = gdt_ptr
   lgdt [eax]        ; load the GDT pointer

   mov ax, 0x10      ; 0x10 is kernel segment data descriptor offset
   mov ds, ax        
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   jmp 0x08:.flush   ; 0x08 is kernel segment code descriptor offset
                     ; far jump to .flush
.flush:
   ret


global idt_flush
idt_flush:
   mov eax, [esp+4]  ; eax = idt_ptr
   lidt[eax]         ; load the IDT ptr
   ret


global load_page_directory
load_page_directory:
   mov eax, [esp+4]
   mov cr3, eax
   ret


global enable_paging
enable_paging:
   mov eax, cr0
   or eax, 0x80000000
   mov cr0, eax
   ret