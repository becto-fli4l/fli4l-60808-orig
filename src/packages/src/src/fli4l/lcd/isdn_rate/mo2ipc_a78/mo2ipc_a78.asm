;------------------------------------------------------------------------------
;  mo2ipc_a78.asm - convert  MATRIX-ORBITAL output from isdn_rate to IPC A78
;
;  Usage: isdn_rate | mo2ipc_a78 > /dev/lcd
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  Compile:
;          nasm -f elf mo2ipc_a78.asm
;          ld -s -o mo2ipc_a78 mo2ipc_a78.o
;    
;  Last Update:  $Id$
;
;  History:
;
;  Arwin Vosselman <arwin@xs4all.nl>   creation                      08.01.2006
;
;
;------------------------------------------------------------------------------

section .data

    id db '$Id$'

    bufsize equ 128
    rowsize equ 20

; command strings
    ; On reset do: reset, WatchDog-off, clock-off, clear
    lcd_reset     db 'M', 0xFF, 'M', 0x2B, 0x00,'M', 0x28, 'M', 0x0D
    lcd_reset_len equ $ - lcd_reset    

    lcd_light_on     db 'M', 0x5E, 0x01
    lcd_light_on_len equ $ - lcd_light_on

    lcd_display_row0     db 'M', 0x0C, 0x00, rowsize
    lcd_display_row0_len equ $ - lcd_display_row0
    lcd_display_row1     db 'M', 0x0C, 0x01, rowsize
    lcd_display_row1_len equ $ - lcd_display_row1

; structs for sys_setitimer
value:
    value.it_interval.tv_secs  dd 0
    value.it_interval.tv_usecs dd 600000    ; 0.6 second
    value.it_value.tv_secs     dd 0
    value.it_value.tv_usecs    dd 600000    ; 0.6 second
    
ovalue:
    ovalue.it_interval.tv_secs  dd 0
    ovalue.it_intersal.tv_usecs dd 0
    ovalue.it_value.tv_secs     dd 0
    ovalue.it_value.tv_usecs    dd 0

; struct for sys_sigaction    

action:
    action.action      dd write
    action.mask        dd 0
    action.flags       dd 0x10000000        ; restart
    action.restorer    dd 0

oldaction:
    oldaction.action   dd write
    oldaction.mask     dd 0
    oldaction.flags    dd 0x10000000
    oldaction.restorer dd 0


section .bss

    ibuffer    resb bufsize
    row0buffer resb rowsize
    row1buffer resb rowsize


section .text

                    ;we must export the entry point to the ELF linker or
    global _start   ;loader. They conventionally recognize _start as their
                    ;entry point.

_start:

    ; set interrupt timer
    mov edx, ovalue
    mov ecx, value
    mov ebx, 0      ; which: REAL timer
    mov eax, 104    ; system call number (sys_setitimer)
    int 0x80        ; call kernel

    ; setup signal action

    mov edx, oldaction 
    mov ecx, action 
    mov ebx, 14     ; sigmum: SIGALRM)
    mov eax, 67     ; system call number (sys_sigaction)
    int 0x80        ; call kernel

    xor   eax, eax
    xor   ebx, ebx    ; counter ibuffer
    call  clear       ; clear rowbuffers

.loop:
    call  getchar     ; read byte from ibuffer

    cmp   al, 0xFE
    je    .control
        stosb         ; put char in rowbuffer
        jmp   .loop

.control:             ; control sequence detected
    call  control
    jmp   .loop

; ===========
; subroutines
; ===========
; read byte from ibuffer

getchar:

    or    ebx,ebx
    jne   .fetch
    call  read       ; read chars from stdin into ibuffer
    
.fetch:

    lodsb             ; ds:si
    dec   ebx         ; counter chars in ibuffer
    ret   

; ===========
;read chars from stdin

read:
    mov   edx, bufsize  ; third argument: buffer size
    mov   ecx, ibuffer  ; second argument: pointer to buffer
    mov   ebx,0         ; first argument: file handle (stdin)
    mov   eax,3         ; system call number (sys_read)
    int   0x80          ; call kernel
    mov   ebx, eax      ; nr. of bytes read
    xor   eax, eax

    mov esi, ibuffer

    ret

; ==========
; write out rowbuffers

write:
    mov   edx, lcd_display_row0_len
    mov   ecx, lcd_display_row0
    call  write2stdout
 
    mov   edx, rowsize    ; third argument: length of buffer
    mov   ecx, row0buffer ; second argument: pointer to buffer
    call  write2stdout

    mov   edx, lcd_display_row1_len
    mov   ecx, lcd_display_row1
    call  write2stdout

    mov   edx, rowsize    ; third argument: length of buffer
    mov   ecx, row1buffer ; second argument: pointer to buffer
    call  write2stdout

    ret

; ===========
; write out to stdout

write2stdout:       ; ecx: pointer to string - edx: length of string
    push ebx
        mov   ebx, 1      ; first argument: file handle (stdout)
        mov   eax, 4      ; system call number (sys_write)
        int   0x80        ; call kernel
    pop ebx

    ret
    
; ===========
; clear rowbuffers

clear:
    mov   al,' '      ; space

    mov   edi, row0buffer
    mov   ecx, rowsize

    rep   stosb

    mov   edi, row1buffer
    mov   ecx, rowsize

    rep   stosb
    
    mov   edi, row0buffer   ; set rowbufferpointer to home

    ret
    
; ===========
; handle control char received

control:
    call  getchar
    cmp   al, 'R'         ; Reset
    jne   .notReset

        mov   edx, lcd_reset_len ; third argument: bytes
        mov   ecx,lcd_reset      ; second argument: pointer to string
        call  write2stdout
        jmp   .return

.notReset:
    cmp   al, 'X'         ; clr home
    jne   .notClear

        call  clear       ; clear rowbuffers only, next write will clear LCD
        jmp   .return

.notClear:
    cmp   al, 'B'         ; Backlight
    jne   .notBacklight

        call  getchar               ; always 0x00 - skip
    ; send Backlight-on to LCD
        mov   edx, lcd_light_on_len ; third argument: bytes
        mov   ecx, lcd_light_on     ; second argument: pointer to string
        call  write2stdout
        jmp   .return

.notBacklight:
    cmp   al, 'P' ; contrast: ignore
    jne   .notContrast

        call  getchar     ; skip value/on-off
        jmp   .return

.notContrast:
    cmp   al, 'G' ; GoTo
    jne   .return       ; command not defined in isdn_rate: just ignore

    call  getchar         ; column
    dec   eax             ; 1-20 -> 0-19
    push  eax

        call  getchar         ; row  1-2 -> 0-1
        cmp   eax, 0x2
        je    .row1

        mov   edi, row0buffer ; row0
        jmp   .add_offset

.row1:
        mov   edi, row1buffer ; row1

.add_offset:
    pop   eax
    add   edi, eax

.return:
    ret
