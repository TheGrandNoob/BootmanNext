align 16

UnsoportedBootPartitionStr db "UnsoportedBootPartition$"

PartitionLabels db "ABCDEFGHIJKLMNOPQRSTUVWXYZ$"
lastLableID dw 0

rootDirStr db '\$'

struc FsExecutor
{
    .addr dw 0
}

lastFsExecutorId dw 0x00

MaxNumOfFsExecutors equ 64

FS_ExecutorTableBegin:
FsExecutorTable: rept MaxNumOfFsExecutors n:0 {FsExecutor_#n#_ FsExecutor}
FS_ExecutorTableEnd:
current_sector dq 0
current_partition_type dw 0
current_partition_disk db 0

partitionTableAddr dw 0
partitionTableSize dw 0
partitionTableSeg dw 0
; ---- partition table ----
; db flags
; dd firtsSector
; dw fsType
; db ID
; db partitionIndex
; db label

partitiontableElementSize equ 10

;CreateFile structure
;dw - PathStrLenght
;dw - Flags


currentPartitionFsType dw 0x00
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

    cmp ax , 0
    jg .IsValidated

    inc dx
    cmp dx , si
    jl @b
.IsValidated:
    ret

disks_init_varables:

    mov [MainDiskID] , dl

    
    ret

disks_init:

    push es bx cx dx  

    mov ax , 40
    call malloc

    mov ax , ( partitiontableElementSize * 16)
    mov [partitionTableSize] , ax
    call malloc 

    mov [partitionTableAddr] , ax
    mov [partitionTableSeg] , es

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
    mov al , [ds:si]
    inc si
    mov [lastLableID] , si

    pop si
    ret

;arg1 - firstSector
;arg2 - disk ID
;arg3 - disk label
;out
;carry if error
attach_partition:

    mov bp , sp
    ccall validate_fs , [ss:bp+10] ,[ss:bp+6] , [ss:bp+2]
    
    cmp cx , 0
    je .end

    mov di , [partitionTableSize]
    add di , [partitionTableAddr]
    mov si , [partitionTableAddr]
    mov bp , [partitionTableSeg]
    mov es , bp

    push ax 
    
    @@:
        mov al , [es:si]
        test al , al
        jz @f

        add si , partitiontableElementSize
        cmp si , di
        jnge @b
    @@:
    pop ax



    mov byte [es:si] , PARTITON_FLAG_MOUTED
    add si , 1
    mov [es:si] , eax
    add si , 4
    mov [es:si] , cx
    add si , 2
    mov [es:si] , dl
    add si , 1
    mov [es:si] , dh

    add si , 1
    cmp bx , 0
    jne @f
    call disks_getLabel    
    mov bh , al
@@:
    mov [es:si] , bh

.end:
    ret
;in
;arg1 - path
;arg2 - flags
;out
;eax - error code
CreateFile:

    mov bp , sp
    mov ax , [ss:bp]

    mov ax , KERNEL_OBJECT_TYPE_FILE
    call AddKernelObject

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

    mov esi , 0
    mov cx , MaxNumOfFsExecutors

    @@:

    ccall fs_executor_exec, esi , FS_EXECUTOR_CMD_VALIDATE,\
    [ss:bp+10] ,[ss:bp+6]

    inc si 
    cmp si , cx
    jl @b
.Found:
.NotFound:
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
    add ax , FS_ExecutorTableBegin
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

    mov bp , sp

    push ax bx cx dx si 

    mov ax , [ss:bp+2]
    mov cx , PTR_SIZE
    mul cx
    add ax , FS_ExecutorTableBegin
    mov si , ax
    mov ax , [ds:si]

    ccall ax , [ss:bp+6]

    pop si dx cx bx ax 

    ret
