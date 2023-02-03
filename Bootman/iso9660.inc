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
;dx - path
iso_chdir:


ret