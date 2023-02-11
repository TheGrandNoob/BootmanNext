
IsoFs_rwBufferAddr dw 0
IsoFs_rwBufferSeg dw 0

IsoFsBufferSize dw 0

;arg1 - function
isofs_executor:

    mov bp , sp 
    mov ebx , [ss:bp+2]
    cmp bx , FS_EXECUTOR_CMD_INIT
    jne @f

    call isofs_init
@@:

    
    cmp bx , FS_EXECUTOR_CMD_FREE
    jne @f

    call isofs_cmd_free
@@:


    cmp bx , FS_EXECUTOR_CMD_VALIDATE
    jne @f

    call isofs_cmd_validate
@@:
    ret

isoFsInitStr db "IsoFsInit",0x0A,0x0D,'$'
isofs_init:

    ret

isoFsFreeStr db "IsoFsFree",0x0A,0x0D,'$'
isofs_cmd_free:
    mov dx , isoFsFreeStr
    mov ah , 0x9
    int 21h
    ret

isoFsValidateStr db "IsoFsValidate",0x0A,0x0D,'$'
isofs_cmd_validate:
    mov dx , isoFsValidateStr
    mov ah , 0x9
    int 21h

    mov eax , [ss:bp]

    ret

ISO_FsDiskRwBuffer rb 2048*2