ISOFS_FILE_STATUS_NOT_FOUND equ 0x1
ISOFS_FILE_STATUS_FOUND equ 0x2
pathLenght dw 0

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

    cmp bx , FS_EXECUTOR_CMD_READ_FILE
    jne @f

    ccall isofs_cmd_read , [ss:bp+6] , [ss:bp+10], [ss:bp+14] , [ss:bp+18]  , [ss:bp+22] 
@@:
    ret
isofs_init:

    mov [IsoFsLastPartitionOffset] , IsoFsPartitions 
    mov [IsoFsLastFileOffset] , IsoFsFiles

    ret

;arg1 - buffer addr (es - segment)
;arg3 - Num of bytes to read
;arg2 - Num of bytes read
;arg4 - data segment
;arg5 - data addr
isofs_cmd_read:
    mov bp , sp
     
    mov eax , [ss:bp+2]
    mov eax , [ss:bp+6]
    mov eax , [ss:bp+10]

    mov eax , dword [ss:bp+14]

    mov eax , [ss:bp+18]
    mov esi , eax
    mov ecx , dword [cs:si + IsoFsFile.Size] 

    mov eax , dword [ss:bp+6]
    cmp ecx , dword [ss:bp+6]
    jle .direct_read

    jmp .buffered_read
.direct_read:


    mov edx , 0
    mov eax , ecx
    mov ebx , 2048
    div ebx

    cmp edx , 0
    jng @f
        inc eax
    @@:
    mov ecx , eax 
    mov eax , [cs:si + IsoFsFile.StartSector]
    mov ebx , [ss:bp+2]
    mov dl , [cs:si + IsoFsFile.diskId] 
    call read_sectors
    jmp .end
.buffered_read:

.end:
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

    pop es
    jne @f

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
;dx - file name
;si - buffer address
;out
;eax - file sector
;ebx - status
;edi - file ptr
IsoFs_SearchInDir:

    push cx dx si
@@:
    cmp byte [cs:si] , 0
    je .NotFound

    push ds es ax
    mov ax , ds
    mov es , ax 

    mov ax , cs 
    mov ds , ax

    pop ax
    add si , 33
    mov di , dx
    call FsFragmentLenght
    call memcmp
    pop es ds
    je .Found
    sub si , 33

    movzx di , byte [cs:si]
    add si , di

    jmp @b
.NotFound:

    mov ebx , ISOFS_FILE_STATUS_NOT_FOUND
    jmp .end

.Found:
    mov eax , [cs:si+2-33]
    mov ebx , ISOFS_FILE_STATUS_FOUND
    sub si , 33
    mov edi , esi
.end:
    pop si dx cx
    ret

isofs_cmd_create:

    xchg bx , bx
    mov bp , sp

    mov esi , [bp+6]
    mov eax , [bp+10]
    mov es , ax 

    mov eax , dword [cs:si]
    mov dl , byte [cs:si+IsoFsPartition.diskId]
    mov ebx , ISO_FsDiskRwBuffer
    mov si , cs
    mov es , si
    mov cx , 2

    push es ax
    mov ax , cs
    mov es , ax 
    pop ax 

    call read_sectors
    
    pop es 

    push es
    mov esi , [bp+6]
    mov eax , [bp+10]
    mov es , ax 

    mov dl , byte [cs:si+IsoFsPartition.diskId]
    pop es

    mov si , ISO_FsDiskRwBuffer
    mov eax , dword [es:si+2+156]

    mov ebx , ISO_FsDiskRwBuffer
    mov si , cs
    mov es , si
    mov cx , 2

    call read_sectors

    mov dx , [bp+2]
    mov si , ISO_FsDiskRwBuffer


    call strlen
    add cx , dx
    mov [cs:pathLenght] , cx
.startSearch:
@@:
    mov si , ISO_FsDiskRwBuffer
    call IsoFs_SearchInDir

    cmp ebx , ISOFS_FILE_STATUS_NOT_FOUND
    je .NotFound
    
    
    call FsFragmentLenght
    add dx , cx
    inc dx
    cmp dx , word [cs:pathLenght]
    
    jge .Found
    
    push ecx
    
    push dx
    mov ebx , ISO_FsDiskRwBuffer
    mov si , cs
    mov es , si
    mov esi , [bp+6]
    mov dl ,  byte [cs:si+IsoFsPartition.diskId]
    mov cx , 2

    
    call read_sectors
    pop dx 

    pop ecx
    jmp @b

.Found:

    mov si , [cs:IsoFsLastFileOffset]
    mov [cs:LastTransactionDataAddr] , si

    mov ecx ,  dword [cs:di+10]
    mov [cs:si + IsoFsFile.Size] , ecx
    mov eax , [cs:di+2]
    mov [cs:si + IsoFsFile.StartSector] , eax
    mov esi , [bp+6]
    mov dl ,  byte [cs:si+IsoFsPartition.diskId]
    
    mov si , [cs:IsoFsLastFileOffset]
    mov [cs:si + IsoFsFile.diskId] , dl

    mov si , [cs:IsoFsLastFileOffset]
    mov [cs:LastTransactionDataAddr] , si
    add si , IsoFsFile_t_sizeof
    mov [cs:IsoFsLastFileOffset] , si

    jmp @f
.NotFound:
    mov eax , 0
    jmp .end
@@:
    mov eax , 1
    jmp .end
.end:
    ret
ISO_FsDiskRwBuffer rb 2048*2