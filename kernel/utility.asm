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


; switch_to_task(process_t* other)
extern previous_task
global switch_to_task_stub
switch_to_task_stub:
   ; -fomit-frame-pointer for this function
   push ebx
   push esi
   push edi
   push ebp

   mov edi, [previous_task]
   mov [edi + 0x18], esp  ; kernel_stack_top = esp

   ; Load the state for the next task
   mov esi, [esp + 20]

   mov esp, [esi + 0x18] ; esp = esi.esp
   mov eax, [esi + 0x1C] ; eax = next task page directory

   ; TODO: maybe change the TSS.ESP0 field to change ring later
; Commenting switching page directories for now
   mov ecx, cr3
   cmp eax, ecx
   je .doneVAS
   mov cr3, eax
.doneVAS:

   pop ebp
   pop edi
   pop esi
   pop ebx

   ret   ; Load next task's EIP from its kernel stack
