align 16
attach_kernel_process:
    ret
init_process_manager:

    push ax bx cx

    pop cx bx ax
    ret


jmpToAdd:
    .addr   dd 0
    .seg    dw 0 

;arg1 - path
exec:

    xchg bx , bx
    mov bp , sp
    
    mov edx , eax
    call memory_allocate_segment
    mov edx , ebx

    mov ah , 0x3C
    mov edx , [bp+2] 
    int 21h 

  
    xor edx , edx
    mov ebx , eax
    mov ah , 0x3F
    mov ecx , 0xFFFF
    int 21h
    pop ebx

    ;mov edx , [bp+2]
    ;ccall CreateFile, edx ,FILE_OPEN_EXISTING


    ;ccall ReadFile , edx , eax , dword 65536 , dword BytesRead
    mov ax , es
    mov bx , 16
    mul bx 
    push ax
    mov cl , 16
    shl eax , cl 
    pop ax 

    mov [jmpToAdd.addr] , 0 
    mov [jmpToAdd.seg] , es
    
    mov bx , es
    mov ds , bx
    mov es , bx
    mov ss , bx
    mov es , bx
    mov sp , 0xFFFF
    jmp 0x3000:0
    
    ret