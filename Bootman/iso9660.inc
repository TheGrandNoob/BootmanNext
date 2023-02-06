iso_current_fsbeginSector dd 0
iso_currentDirSize dd 0
iso_currentDirSector dd 0

iso_nextFileOffset dw 0


;in
;dl - diskIndex
;eax - sector
;out
;equal if iso
iso_validate:
    push es bp bx cx dx si di
    mov bp , cs
    mov es , bp

    mov bx , KRNL_DISK_IO_BUFFER
    mov cx , 1

    call read_sectors

    mov dx , ISO_Str
    call strlen

    mov si , ISO_Str
    mov di , KRNL_DISK_IO_BUFFER+1
    call memcmp

    rep cmpsb
    
    pop di si dx cx bx bp es

    ret

;in
;eax - start sector
iso_init_partition:
    mov [iso_current_fsbeginSector] , eax
    ret

;in
;out 
;carry
;eax - primary volume discriptor 
isoSearchPrimaryVolume:

    push bp es bx 
@@:
    mov bp , ds
    mov es , bp

    mov bx , KRNL_DISK_IO_BUFFER
    mov cx , 1
    
    call read_sectors
    cmp [KRNL_DISK_IO_BUFFER] , 0x1
    je .end
    cmp [KRNL_DISK_IO_BUFFER] , 255
    je .error
    inc eax
    jmp @b
.error:
    stc
.end:
    pop bx es bp
    
    ret

;in
;dx - path
iso_chdir:

    call strlen
    cmp cx , 1
jne @f
    mov si , dx
    mov ah , [ds:si]
    cmp ah , '/'

    mov eax , iso_current_fsbeginSector
    call isoSearchPrimaryVolume

    jc .error

    mov bp , ds
    mov es , bp
    mov bx , KRNL_DISK_IO_BUFFER
    mov cx , 1
    call read_sectors

    add si , 2
    mov eax , [ds:si]

    mov [iso_currentDirSector] , eax
    
    add si , 8
    mov eax , [ds:si]

    mov [iso_currentDirSize] , eax

    mov bp , ds
    mov es , bp
    mov bx , KRNL_DISK_IO_BUFFER
    mov cx , 1
    call read_sectors

    jne .error
@@:
.error:
    ret

;in
;out
;ax - flags
;dx - name
iso_nextfile:
    ret


searchInDir:
    ret

;in
;DX - path
;eax - buffer size
;ebx - start from
;es - buffer segment
;edi - buffer addr
iso_read:
    ret
