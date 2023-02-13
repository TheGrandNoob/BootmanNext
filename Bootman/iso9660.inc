
IsoFs_indifier db "CD001" , 0x00

IsoFsPartitionsTable rd 4
IsoFsLastPartitionOffset dw 0

;arg1 - function
isofs_executor:

    mov [LastTransactionDataAddr] , word 0
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

    ccall isofs_cmd_validate , [ss:bp+6] , [ss:bp+10]
@@:

    cmp bx , FS_EXECUTOR_CMD_ATTACH_PARTITION
    jne @f

    ccall isofs_cmd_attach , [ss:bp+6] , [ss:bp+10]
@@:


    ret
isofs_init:

    mov [IsoFsLastPartitionOffset] , IsoFsPartitionsTable 

    ret

isoFsFreeStr db "IsoFsFree",0x0A,0x0D,'$'
isofs_cmd_free:
    mov dx , isoFsFreeStr
    mov ah , 0x9
    int 21h
    ret

isoFsValidateStr db "IsoFsValidate",0x0A,0x0D,'$'
isofs_cmd_validate:

    mov eax , 0

    mov bp , sp

    push es

    mov ax , ds
    mov es , ax

    mov eax , [bp+2]
    mov cx , 2
    mov bx , ISO_FsDiskRwBuffer
    mov edx , [bp+6]
    
    call read_sectors    

    mov si , IsoFs_indifier
    mov di , ISO_FsDiskRwBuffer+1
    mov cx , 5
    call memcmp

    jne @f
    mov eax , 1

    mov [LastTransactionDataSeg] , ds
    mov si , word [IsoFsLastPartitionOffset]
    add si ,4
    mov [IsoFsLastPartitionOffset] , si
    sub si , 4 
@@:
    pop es
    ret

;arg1 - diskID
;arg2 - fsStartSector
;arg3 - data seg
;arg4 - data addr
isofs_cmd_attach:
    mov eax , 0

    mov bp , sp

    push es

    mov ax , ds
    mov es , ax

    mov eax , [bp+2]
    mov cx , 2
    mov bx , ISO_FsDiskRwBuffer
    mov edx , [bp+6]
    
    call read_sectors    

    mov si , IsoFs_indifier
    mov di , ISO_FsDiskRwBuffer+1
    mov cx , 5
    call memcmp

    jne @f
    mov eax , 1
@@:
    pop es

    mov si , word [IsoFsLastPartitionOffset]
    mov eax , [bp+2]
    mov [IsoFsLastPartitionOffset] , ax
    add si , 4
    mov [IsoFsLastPartitionOffset] , si

    ret
ISO_FsDiskRwBuffer rb 2048*2