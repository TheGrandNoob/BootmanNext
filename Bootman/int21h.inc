
int21h_init:
    pusha 
    mov si , 0x21*4
    mov bp , 0
    mov es , bp

    mov bp , cs
    mov [es:si] , word int21h
    add si , 2
    mov [es:si] , cs
    popa
    ret

Int21h_BytesRead dd 0
int21h:
    cmp ah , 0
    jne @f
@@:

    cmp ah , 0x9
    call int21h_display_string
    jne @f
@@:
    cmp ah , 0x3C
    
    push ebx
    xor ebx , ebx
    mov bl , al
    xor eax , eax 
    mov eax , ebx 
    pop ebx

    ccall CreateFile , edx , eax
    jne @f
@@:
    cmp ah , 0x3D
    push ebx
    xor ebx , ebx
    mov bl , al
    xor eax , eax 
    mov eax , ebx 
    pop ebx

    ccall CreateFile , edx , eax
    jne @f
@@:
    cmp ah , 0x3F
    ccall ReadFile , ebx , edx , ecx , Int21h_BytesRead
    jc @f
    mov eax , [Int21h_BytesRead]
    jne @f
@@:

    cmp ah , 0x63
    push ds 
    mov ax , cs
    mov ds , ax
    mov ax , [currentPartition]
    mov dl , [MainDiskID]
    pop ds
    jne @f
@@:


    iret

int21h_display_string:
    push si ax
    mov ah , 0xE
    mov si , dx
@@:        
    mov al , [ds:si]
    cmp al , '$'
    je @f
    int 10h
    inc si
    jmp @b
@@:
    pop ax si 
    ret