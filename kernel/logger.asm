extern put_serial  ; takes a character and prints it

global print_serial_hex
;; Routine to print hex value by Charles A. Crayne
;; Adapted to work with current primitives.
print_serial_hex:
;hex to ascii routines
;esp + 4 -- value to be converted
        mov     eax, [esp + 4]
hexdd:  push    eax
        shr     eax,16          ;do high word first
        call    hexdw
        pop     eax
hexdw:  push    eax
        shr     eax,8           ;do high byte first
        call    hexdb
        pop     eax
hexdb:  push    eax
        shr     eax,4           ;do high nibble first
        call    hexdn
        pop     eax
hexdn:  and     eax,0fh         ;isolate nibble
        add     al,'0'          ;convert to ascii
        cmp     al,'9'          ;valid digit?
        jbe     hexdn1          ;yes
        add     al,7            ;use alpha range
hexdn1: push    ax
        call    put_serial
        pop     ax
        ret