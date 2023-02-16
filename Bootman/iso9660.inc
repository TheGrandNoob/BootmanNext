
IsoFs_indifier db "CD001" , 0x00

IsoFsPartitionsTable rd 16
IsoFsLastPartitionOffset dw 0

struc IsoFsPartition_t
{
    .fsStartSector      dd ?
    .diskId             db ?
    .sizeInSectors      db ?
}
virtual at 0
    IsoFsPartition IsoFsPartition_t
    IsoFsPartition_t_sizeof:
end virtual

struc IsoFsFile_t
{
    .StartSector        dd ?
    .Size               dd ?
    .diskId             db ?
}

IsoFsLastFileOffset dw 0x00
virtual at 0
    IsoFsFile IsoFsFile_t
    IsoFsFile_t_sizeof:
end virtual

IsoFsFiles: rept 64 n:0 {IsoFsFiles_#n#_ IsoFsFile_t}
IsoFsPartitions: rept 16 n:0 {IsoFsPartition_#n#_ IsoFsPartition_t}
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

    cmp bx , FS_EXECUTOR_CMD_CREATE
    jne @f

    ccall isofs_cmd_create , [ss:bp+6] , [ss:bp+10], [ss:bp+14]
@@:
    ret
isofs_init:

    mov [IsoFsLastPartitionOffset] , IsoFsPartitions 

    ret


isofs_cmd_free:

    ret

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
    mov cx , 1
    mov bx , ISO_FsDiskRwBuffer
    mov edx , [bp+6]
    
    call read_sectors    

    mov si , IsoFs_indifier
    mov di , ISO_FsDiskRwBuffer+1
    mov cx , 5
    call memcmp

    jne @f
    pop es

    mov si , word [IsoFsLastPartitionOffset]
    mov eax , [bp+2]
    mov edx , [bp+6]

    mov dword [si+IsoFsPartition.fsStartSector] , eax
    mov byte [si+IsoFsPartition.diskId] , dl

    mov [LastTransactionDataAddr] , si
    mov [LastTransactionDataSeg] , ds

    add si , IsoFsPartition_t_sizeof
    mov [IsoFsLastPartitionOffset] , si
    mov eax , 1

@@:
    ret

;in
;ds - str seg
;dx - str
;out 
;cx - lenght
IsoFs_FragmentLenght:
    push si dx ax 

    mov cx , 0
    mov si , dx
@@:
    mov al , [ds:si]
    cmp al , '$'
    je .end
    cmp al , 0
    je .end
    cmp al , '\'
    je .end

    inc si 
    inc cx

    jmp $b
.end:
    pop ax dx si
    ret


isofs_cmd_create:

    mov sp , bp


    mov esi , [bp+10]
    mov eax , [bp+14]
    mov es , ax 

    mov eax , dword [es:si]
    mov dl , byte [es:si+IsoFsPartition.diskId]
    mov ebx , ISO_FsDiskRwBuffer
    mov si , cs
    mov es , si
    mov cx , 2


    call read_sectors

    push es
    mov esi , [bp+10]
    mov eax , [bp+14]
    mov es , ax 

    mov dl , byte [es:si+IsoFsPartition.diskId]
    pop es



    mov si , ISO_FsDiskRwBuffer
    mov eax , dword [es:si+2+156]

    mov ebx , ISO_FsDiskRwBuffer
    mov si , cs
    mov es , si
    mov cx , 2

    call read_sectors

    mov dx , [bp+2]
    call strlen
    mov bx , cx
    mov dx , [bp+2]


    jg @b

    mov si , [IsoFsLastFileOffset]
    mov [LastTransactionDataAddr] , si


    mov si , [IsoFsLastFileOffset]
    mov [LastTransactionDataAddr] , si
    add si , IsoFsFile_t_sizeof

    ret
ISO_FsDiskRwBuffer rb 2048*2