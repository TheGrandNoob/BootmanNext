align 16
;in
;dx - string
;out
;cx - lenght 
strlen:
    push ax dx si 
    mov si , dx
    mov dx , 0

@@:
    mov al , [ds:si]

    cmp al , '$'
    je .end
    cmp al , 0
    je .end
    inc si
    inc dx
    jmp @b
.end:
    mov cx , dx
    pop si dx ax
    ret


;in
;dx - string
;ah - specific symbol
;out
;cx - lenght 
strlen_ssymbol:
    push ax dx si 
    mov si , dx
    mov dx , 0

@@:
    mov al , [ds:si]

    cmp al , ah
    je .end
    inc si
    inc dx
    jmp @b
.end:
    mov cx , dx
    pop si dx ax
    ret