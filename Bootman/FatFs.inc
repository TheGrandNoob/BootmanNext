
;arg1 - function
fatfs_executor:

    mov bp , sp 
    mov ebx , [ss:bp+2]
    cmp bx , FS_EXECUTOR_CMD_INIT
    jne @f

    call fatfs_init
@@:

    
    cmp bx , FS_EXECUTOR_CMD_FREE
    jne @f

    call fatfs_cmd_free
@@:


    cmp bx , FS_EXECUTOR_CMD_VALIDATE
    jne @f

    call fatfs_cmd_validate
@@:
    ret

fatfsInitStr db "FatFsInit",0x0A,0x0D,'$'
fatfs_init:
    mov dx , fatfsInitStr
    mov ah , 0x9
    int 21h
    ret

fatfsFreeStr db "FatFsFree",0x0A,0x0D,'$'
fatfs_cmd_free:
    mov dx , fatfsFreeStr
    mov ah , 0x9
    int 21h
    ret

fatfsValidateStr db "FatFsValidate",0x0A,0x0D,'$'
fatfs_cmd_validate:
    mov dx , fatfsValidateStr
    mov ah , 0x9
    int 21h

    ret