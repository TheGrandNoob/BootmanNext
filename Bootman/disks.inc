align 16
include "iso9660.inc"

EXECUTOR_CMD_TYPE_READ_FILE equ 0x2
UnsoportedBootPartitionStr db "UnsoportedBootPartition$"

PartitionLabels db "ABCDEFGHIJKLMNOPQRSTUVWXYZ$"
lastLableID dw 0

rootDirStr db '\$'

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

    push cx bx bp si di

    mov cx  , 1
    mov bx , DISK_RW_BUFFER
    mov bp , ds 
    mov es , bp

    call read_sectors


    mov si , 0x036+DISK_RW_BUFFER
	mov di , FAT12_Str
	rep cmpsb
    jne @F

    mov ax , BPB_TYPE_FAT12
    jmp .end
@@:

	mov di , FAT16_Str
	rep cmpsb
    jne @F

    mov ax , BPB_TYPE_FAT16
    jmp .end
@@:

	mov di , FAT32_Str
	rep cmpsb
    jne @F

    mov ax , BPB_TYPE_FAT32
    jmp .end
@@:
    mov si , 0x1
	mov di , ISO_Str
	rep cmpsb
    jne @F

    mov ax , BPB_TYPE_ISO
    jmp .end
@@:
.end:

    pop di si bp bx cx

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

    mov eax , 16
    mov dl , [MainDiskID] 
    call validate_fs
    jc .end
    mov bh , 'A'
    call attach_partition

    mov dx , 0
    call setCurrentParttion

    mov ax , KERNEL_OBJECT_TYPE_FILE
    mov bx , 0x200
    mov cx , disks_kernelObjectExecutor
    call AddKernelObjectExecuter


.end:
    pop dx cx bx es

    ret


;in
;eax - partition sector
;dl - disk id
;out
;cx - partion fstype
validate_fs:

    push eax

    mov cx , 0
    call iso_validate
    jne @f
    mov cx , BPB_TYPE_ISO
@@:
.end:
    pop eax
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

;eax - firstSector
;dl - disk ID
;dh - partitionIndex, 255 if mbr not present
;bl - flags
;bh - label automaticly select if zero
;out
;carry if error
attach_partition:

    call validate_fs
    
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
;DX - partition index
;out
;carry if error
;eax - error code
setCurrentParttion:
    push si es ax bx

    mov di , [partitionTableAddr]
    mov bp , [partitionTableSeg]
    mov es , bp

    mov ax , [partitiontableElementSize]
    mov bx , dx
    mul bx

    add di , ax
    add di , 1+4
    mov bx , word [es:di] 
    mov [currentPartitionFsType] , bx

    mov si , currentPath

    add di , 4
    mov al , byte [es:di]
    mov [ds:si] , al
    inc si
    mov [ds:si] , byte ':'
    inc si
    mov [ds:si] ,byte  '\'
    inc si
    mov [ds:si] ,byte  '$'
    
    pop bx ax es si
    ret



;in
;ax - handle
;bx - buffer addr
;cx - buffer seg
;out
;carry
;ax - error code
ReadFile:

    mov bx , EXECUTOR_CMD_TYPE_READ_FILE
    call KernelObjectExecute
    ret

;in
;dx - path
;bx - flags
;out
;carry
;ax - error code
CreateFile:

    mov bp , sp
    mov ax , [ss:bp]

    mov ax , KERNEL_OBJECT_TYPE_FILE
    call AddKernelObject

    ret


KOBjExecutorStr db "called disks executor",0x0A,0x0D,'$'
disks_kernelObjectExecutor:

    mov dx , KOBjExecutorStr
    mov ah , 0x9
    int 21h
    retf