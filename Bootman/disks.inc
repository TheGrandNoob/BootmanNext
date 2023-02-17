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

    push es bx cx dx  

    mov ax , 40
    call malloc

    mov ax , KERNEL_OBJECT_TYPE_FILE
    mov bx , 0x200
    mov cx , disks_kernelObjectExecutor
    call AddKernelObjectExecuter

.end:
    pop dx cx bx es

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
    
    
    mov byte [si + Partition.flags] , PARTITON_FLAG_MOUTED
    mov word [si + Partition.fsType] , ax

    mov ax , [LastTransactionDataAddr]
    mov word [si + Partition.fsInfoAddr] , ax
    mov ax , [LastTransactionDataSeg]
    mov word [si + Partition.fsInfoSeg] , ax
    
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

ReadFile:
    ret
;in
;arg1 - path
;arg2 - flags
;out
;eax - handle
CreateFile:

    mov bp , sp

    mov si , [bp+2]

    cmp [es:si+1] , byte ':'
    je .isGlobalPathWithDisk

.isGlobalPathWithDisk:  

    mov edi , FsPartitionTable
    add edi , dword [currentPartition]
    mov esi , 0
    mov si , word [ds:edi + Partition.fsType]
    mov eax , 0
    mov ax , word [ds:edi + Partition.fsInfoAddr]
    mov edx , 0
    mov dx , word [ds:edi + Partition.fsInfoSeg]

    mov ebx , [bp+2]
    inc ebx
    ccall fs_executor_exec, esi, FS_EXECUTOR_CMD_CREATE , ebx , eax , edx
    cmp eax , 0
    je .end


    mov ax , [ss:bp]

    mov ax , KERNEL_OBJECT_TYPE_FILE
    call AddKernelObject
.end:
    ret

ReadFileStr db "ReadFile",0x0A,0x0D,'$'

FreeFileHandleStr db "FreeFileHandle",0x0A,0x0D,'$'
disks_kernelObjectExecutor:

    cmp bx , 0x1
    jne @f

    mov dx , FreeFileHandleStr
    mov ah , 0x9
    int 21h   
@@:

    cmp bx , EXECUTOR_CMD_TYPE_READ_FILE
    jne @f

    mov dx , ReadFileStr
    mov ah , 0x9
    int 21h   
@@:

    retf

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
    mov di , [ds:si]


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

    pop es di si dx cx bx
    pop bp 

    ret
