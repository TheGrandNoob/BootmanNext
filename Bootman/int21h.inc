
int21h_init:
    mov si , 0x21*4
    mov bp , 0
    mov es , bp

    mov bp , cs
    mov [es:si] , word int21h
    add si , 2
    mov [es:si] , cs
    ret


int21h:
    cmp ah , 0
    jne @f
@@:

    cmp ah , 0x9
    call int21h_display_string
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