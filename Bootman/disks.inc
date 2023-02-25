align 16

UnsoportedBootPartitionStr db "UnsoportedBootPartition$"

PartitionLabels db "ABCDEFGHIJKLMNOPQRSTUVWXYZ$"
lastLableID dw 0
struc FsExecutor
{
    .addr dw 0
}

LastTransactionDataAddr dw 0x0
LastTransactionDataSeg dw 0x0

struc Partition_t
{
    .flags      db ?
    .fsType     dw ?
    .label      db ?

    .fsInfoAddr dw ?
    .fsInfoSeg  dw ?
}
virtual at 0
  Partition Partition_t
  PartitionElSize:
end virtual

struc File_t
{
    .fsType dw ?
    .flags dw ?
    .fsInfoAddr dw ?
    .fsInfoSeg  dw ?
}

LastFileIndex dw 0

virtual at 0
  FileT File_t
  FileSize:
end virtual

lastFsExecutorId dw 0x00
LastPartitionID dw 0x00

MaxNumOfFsExecutors equ 64
MaxNumOfPartitions equ 32

FsExecutorTableBegin:
FsExecutorTable: rept MaxNumOfFsExecutors n:0 {FsExecutor_#n#_ FsExecutor}
FsExecutorTableEnd:

FsPartitionTableBegin:
FsPartitionTable: rept MaxNumOfPartitions n:0 {Partition_#n#_ Partition_t}
FsPartitionTableEnd:

FilesTableBegin:
FilesTable: rept 64 n:0 {File_#n#_ File_t}
FilesTableEnd:

    include 'CreateFile.inc'
dapack:
    	dapack_size:    db 0x10
    	dapack_null:    db 0x00
    	dapack_nblocks: dw 0
    	dapack_offset:  dw 0
    	dapack_segment: dw 0
    	dapack_LBA:     dq 0

;in
;eax - lba
;cx - num of sectors
;bx - offset
;es - segment
read_sectors:
    	pusha
        push eax
        mov ax , cs
        mov ds , ax
        pop eax

    	mov dword [dapack_LBA], eax
    	mov word  [dapack_nblocks], cx
    	mov word  [dapack_offset], bx
    	mov word  [dapack_segment], es

    	mov ah, 0x42
    	mov si, dapack
    	int 0x13
    	popa
    	ret

;IN
;DL - disk id
;EAX - sector
;OUT ax - bpbType
disks_getbpbType:

    mov dx , 0 
    mov si , [lastFsExecutorId]

    @@:

    mov ax , dx
    mov bx , FS_EXECUTOR_CMD_VALIDATE
    call fs_executor_exec

    cmp eax , 0
    jne .IsValidated

    inc dx
    cmp dx , si
    jl @b
.IsValidated:
    ret
FsFragmentLenght:
    push ax dx si 
    mov si , dx
    mov dx , 0

@@:
    mov al , [ds:si]

    cmp al , '\'
    je .end
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

disks_init_varables:

    mov [MainDiskID] , dl
    mov [BootPartitionSector] , eax
    
    ret

disks_init:

    pusha

.end:
    popa

    ret


;out
;al - label
disks_getLabel:
    push si

    mov si ,  [lastLableID]
    inc si
    mov [lastLableID] , si
    dec si

    add si , PartitionLabels
    mov al , [ds:si]

    pop si
    ret

;arg1 - firstSector
;arg2 - disk ID
;arg3 - disk label
;out
;carry if error
attach_partition:

   
    mov bp , sp

    mov esi , 0
    mov cx , MaxNumOfFsExecutors

    @@:

    ccall fs_executor_exec, esi , FS_EXECUTOR_CMD_ATTACH_PARTITION,\
    [ss:bp+2] ,[ss:bp+6]

    cmp eax , 1
    je .Found

    inc si 
    cmp si , cx
    jl @b
    jmp .NotFound
.Found:
    push ax bx
    mov si , FsPartitionTable
    
    @@:
        mov al , [es:si]
        test al , al
        jz @f

        add si , PartitionElSize
        cmp si , di
        jnge @b
    @@:
    pop bx ax
    
    
    mov byte [cs:si + Partition.flags] , PARTITON_FLAG_MOUTED
    mov word [cs:si + Partition.fsType] , ax

    mov ax , [cs:LastTransactionDataAddr]
    mov word [cs:si + Partition.fsInfoAddr] , ax
    mov ax , [cs:LastTransactionDataSeg]
    mov word [cs:si + Partition.fsInfoSeg] , ax
    
    cmp bh , [ss:bp+10]
    jne @f
    call disks_getLabel    
@@:
    mov byte [si + Partition.label] , al
    mov [currentPath] , al

    mov [currentPath+1] , ':'
    mov [currentPath+2] , '\'
    mov [currentPath+3] , '$'
    mov eax , 0
    jmp .end
.NotFound:
    stc
.end:
    ret


;in
;arg1 - handle
;arg2 - buffer address (es:buffer segment)
;arg3 - Num of bytes to read
;arg4 - Num of bytes read
;out
;eax - status
ReadFile:
    mov bp , sp
    push ebx ecx edx

    mov eax , [bp+2]
    mov bx , FileSize
    mul bx

    mov esi , 0
    mov si , ax 
    
    mov eax , 0
    mov ebx , 0
    
    mov ax , word [cs:si + FilesTableBegin + FileT.fsInfoSeg]
    mov bx , word [cs:si + FilesTableBegin + FileT.fsInfoAddr]
    mov ecx , 0
    mov cx ,[cs:si + FilesTableBegin + FileT.fsType]

    ccall fs_executor_exec, ecx , FS_EXECUTOR_CMD_READ_FILE,[ss:bp+6],[ss:bp+10],[ss:bp+14],eax, ebx

    pop edx ecx ebx 
    ret


;in
;arg1 - fs start sector
;arg2 - disk logical id
;out
;bool eax
validate_fs:

    mov bp , sp

    push esi ebx ecx
    mov esi , 0
    mov cx , MaxNumOfFsExecutors

    @@:

    ccall fs_executor_exec, esi , FS_EXECUTOR_CMD_VALIDATE,\
    [ss:bp+6],[ss:bp+2]

    cmp eax , 1
    je .Found

    inc si 
    cmp si , cx
    jl @b
    jmp .NotFound
.Found:
    mov eax , 1
.NotFound:

    pop ecx ebx esi
    ret

;in
;arg1 - addr
;out
;ax - exec id
attach_fs_executor:

    mov bp , sp
    
    push si es ax dx cx

    xor dx , dx

    mov ax , [lastFsExecutorId]
    inc ax 
    mov [lastFsExecutorId] , ax
    dec ax 
    
    mov bx , PTR_SIZE
    mul bx
    add ax , FsExecutorTableBegin
    mov si , ax

    mov ax , [ss:bp+2]
    mov [ds:si] , ax

    mov esi , 0
    mov si , [lastFsExecutorId]
    dec si
    ccall fs_executor_exec, esi, FS_EXECUTOR_CMD_INIT

    pop cx dx ax es si

    ret

;in
;arg1 - execuotor id 
;arg2 - cmd
fs_executor_exec:

    push bp
    mov bp , sp
    add bp , 2

    push bx cx dx si di es

    mov ax , [ss:bp+2]
    mov cx , PTR_SIZE
    mul cx
    add ax , FsExecutorTableBegin
    mov si , ax
    mov di , [cs:si]


    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_INIT
    jne @f
        ccall di , [ss:bp+6]
    @@:

    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_VALIDATE
    jne @f
        ccall di , [ss:bp+6] , [ss:bp+10] , [ss:bp+14]
    @@:

    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_ATTACH_PARTITION
    jne @f
        ccall di , [ss:bp+6] , [ss:bp+10] , [ss:bp+14]
    @@:

    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_CREATE
    jne @f
        ccall di , [ss:bp+6] , [ss:bp+10] , [ss:bp+14] , [ss:bp+18]
    @@:

    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_FREE_FILE
    jne @f
        ccall di , [ss:bp+6]
    @@:

    cmp dword [ss:bp+6] , FS_EXECUTOR_CMD_READ_FILE
    jne @f
        ccall di , [ss:bp+6] , [ss:bp+10] , [ss:bp+14] , [ss:bp+18], [ss:bp+22] , [ss:bp+26]
    @@:

    pop es di si dx cx bx
    pop bp 

    ret
