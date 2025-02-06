[org 0x7c00]

; Bootloader Start - Print Simple Messages
mov ah, 0x0e
mov al, 'H'
int 0x10

mov al, 'e'
int 0x10

mov al, 'l'
int 0x10

mov al, 'l'
int 0x10

mov al, 'o'
int 0x10

mov al, ','
int 0x10

mov al, ' '
int 0x10

mov al, 'O'
int 0x10

mov al, 'c'
int 0x10

mov al, 'e'
int 0x10

mov al, 'a'
int 0x10

mov al, 'n'
int 0x10

mov al, 'O'
int 0x10

mov al, 'S'
int 0x10

jmp 0x1000

times 510-($-$$) db 0
dw 0xAA55