
ShutdownCmd_str db "shutdown",0x00
UnkownCmdStr db "Unkown cmd: $"

parse_cmd:

    mov eax , 0

    mov si , cmdLine
	mov di , ShutdownCmd_str
	rep cmpsb
    jne @F

    mov eax , 1
    jmp .end
    @@:
    
    mov ah , 0x9
    mov dx , UnkownCmdStr
    int 21h 


    mov ah , 0xE
    mov si , cmdLine
@@:
    mov al , [ds:si]
    int 10h
    inc si
    cmp al , 0
    jne @b

    mov ah , 0xE
    mov al , 0x0A
    int 10h

    mov ah , 0xE
    mov al , 0x0D
    int 10h

    mov eax , 0
.end:
    ret

cmd_b_cur dw 0

shell_readkb:

    mov ah , 0
    int 16h

    mov si , [cmd_b_cur]
    mov [cmdLine+si], al
    inc si
    mov [cmd_b_cur] , si

    cmp eax , 0x1C0D
    jne @f

    mov ah , 0xE
    mov al , 0x0A
    int 10h

    mov ah , 0xE
    mov al , 0x0D
    int 10h

    mov si , [cmd_b_cur]
    add si , 1
    mov byte [ds:si] , 0x00

    mov [cmd_b_cur] , 0
    mov eax , 1
    jmp .end
@@:  

    mov ah , 0xE
    int 10h
.end:
    ret


shell_task:

    mov bp , ds
    mov es , bp

    @@:

    call shell_readkb
    cmp eax , 1
    jne .noCmd
    call parse_cmd

.noCmd:

    cmp eax , 1
    je .end
    jmp @b
.end:
    ret