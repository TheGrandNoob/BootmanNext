include "iso9660.inc"

UnsoportedBootPartitionStr db "UnsoportedBootPartition$"

PartitionLabels db "ABCDEFGHIJKLMNOPQRSTUVWXYZ$"
lastLableID dw 0

current_sector dq 0
current_partition_type dw 0
current_partition_disk db 0

partitionTableAddr dw 0
partitionTableSize dw 0
partitionTableSeg dw 0
; ---- partition table ----
; db flags
; dq firtsSector
; dw fsType
; db ID
; db partitionIndex
; db label

partitiontableElementSize equ 10

align 8
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
    jc @f
    mov bh , 'A'
    call attach_partition
@@:
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

; ---- partition table ----
; db flags
; dq firtsSector
; dw fsType
; db ID
; db partitionIndex
; db label