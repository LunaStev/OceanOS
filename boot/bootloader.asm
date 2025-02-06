[ORG 0x7C00]

    ; 화면에 "Hello, OceanOS" 출력
    mov si, msg
    call print_string

    ; GDT 설정
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_protected_mode  ; Far jump로 CS 업데이트

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

gdt:
    dq 0x0000000000000000          ; Null Descriptor
    dq 0x00CF9A000000FFFF          ; Code Segment (Base=0, Limit=4GB)
    dq 0x00CF92000000FFFF          ; Data Segment (Base=0, Limit=4GB)

gdt_descriptor:
    dw $ - gdt - 1
    dd gdt

[BITS 32]
init_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7C00                ; 스택 포인터 설정

    mov esi, protected_msg
    mov edi, 0xB8000               ; VGA 텍스트 버퍼 주소
    mov ah, 0x0F                   ; 속성 (흰색 텍스트)
.loop:
    lodsb
    cmp al, 0
    je .done
    mov [edi], ax
    add edi, 2
    jmp .loop
.done:
    hlt

msg db "Hello, OceanOS", 0
protected_msg db "Protected Mode!", 0

CODE_SEG equ 0x08
DATA_SEG equ 0x10

times 510-($-$$) db 0
dw 0xAA55