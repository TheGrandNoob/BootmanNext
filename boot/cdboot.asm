org 0x7c00
use16

jmp start
nop

times 8-($-$$) db 0
boot_info:
    bi_PVD      dd 0
    bi_boot_LBA dd 0
    bi_boot_len dd 0
    bi_checksum dd 0
    bi_reserved rb 40

times 90-($-$$) db 0

start:
        mov [bootDiskId] , dl
	cli
    	cld
    	jmp 0x0000:.initialise_cs
  	.initialise_cs:
    	xor si, si
    	mov ds, si
    	mov es, si
    	mov ss, si
    	mov sp, 0x7c00
    	sti

    	; check disk extension support
    	mov ah, 0x41
    	mov bx, 0x55aa
    	int 0x13
    	jc boot_error.notSupported
    	cmp bx, 0xaa55
    	jne boot_error.notSupported
	
	mov eax , [bi_boot_LBA]
@@:
	mov dl ,[bootDiskId]
	add eax , 16
	mov cx , 1
	mov bx , 0x500 
	call read_2k_sectors

	mov dh , 0x1
	cmp dh , byte [DISK_RWBUFFER]
	jne @b

        mov eax , [bi_boot_LBA]
	add eax , [DISK_RWBUFFER+140]
	mov cx , 1
	mov bx , 0x500 
	call read_2k_sectors
	
	jmp $
boot_error:
.notSupported:
	mov si , NotSupportedStr
        call display_string
        xor ah , ah
        int 16h
        int 18h

NotSupportedStr db "NotSupported",0x00

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
; On entry:
;   ds:si -> asciiz string to display
;
display_string:
           cld
           mov ah , 0xE
@@:        lodsb
           test   al,al

           jz   short @f
           int 10h
           jmp @b
@@:
           ret

	jmp $
; --- Read sectors from disk ---
; IN:
; eax <- start LBA (2k sectors)
; cx <- number of 2k sectors
; dl <- drive number
; ds <- ZERO
; bx <- buffer offset
; es <- buffer segment

; OUT:
; Carry if error

bootDiskId db 0x00

align 8
dapack:
    	dapack_size:    db 0x10
    	dapack_null:    db 0x00
    	dapack_nblocks: dw 0
    	dapack_offset:  dw 0
    	dapack_segment: dw 0
    	dapack_LBA:     dq 0

read_2k_sectors:
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
org 0x500

DISK_RWBUFFER: