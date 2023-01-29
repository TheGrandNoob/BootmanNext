UnsoportedBootPartitionStr db "UnsoportedBootPartition$"

align 8
dapack:
    	dapack_size:    db 0x10
    	dapack_null:    db 0x00
    	dapack_nblocks: dw 0
    	dapack_offset:  dw 0
    	dapack_segment: dw 0
    	dapack_LBA:     dq 0

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

disks_init:

    mov [MainDiskID] , dl
    
    ;eax - num of drives
    mov bx , 0x40
    mov es , bp
    mov ax , 0
    mov ah , byte [es:75]

    mov si , ax

    mov eax , 0 
    mov dl , [MainDiskID]
    call disks_getbpbType

    cmp eax , 0
    jne @F

    mov eax , 16 
    mov dl , [MainDiskID]
    call disks_getbpbType

    cmp eax , 0
    jne @F

.error:
    mov dx , UnsoportedBootPartitionStr
    call int21h_display_string

    jmp $
    @@:

    mov byte [partitions + 2*'C' ] , dl
    cmp eax , 0
    jne @F
    mov byte [partitions + 2*'C' +1] , 0
@@:
    mov byte [partitions + 2*'C' +1] , dh
    mov word [partitions + 2*'C' +2] , ax
    ret


; DX - path
disks_chdir:

    push ax si
    mov si , dx
    inc si
    mov al , byte [ds:si]
    dec si

    cmp al , ':'
    jne @F

    mov al , [ds:si]
    mov [currentPath] , al
    

@@:

.end:
    pop si ax

    ret
