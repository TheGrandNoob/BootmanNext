org 0x7c00

jmp main
DriveNum db  0x00
BootedSector dd 0x00
main:
        RESVD_SECTS  equ  32
        NUM_FATS equ 2
        STACK_OFFSET  equ 4400h

        mov [DriveNum] , dl
        cli

        mov bp , 0
        mov es , bp
        mov ds , bp
        mov ss , bp
        mov sp , STACK_OFFSET

        pushf
        push 0F000h
        popf
        pushf
        pop  ax
        and  ax,0F000h

        popf
        sti

        mov  ah,41h
        mov  bx,55AAh
        int  13h                     ; dl still = drive_num
        jc   short @f
        shr  cx,1                    ; carry = bit 0 of cl
        adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
        jz   short read_remaining

bootErr:
        mov si , BootErrMsg
        call display_string
        xor  ah,ah              ; Wait for keypress
        int  16h
        int  18h

read_remaining:

        ;mov ebx , 0x7E00
        ;mov cx , 10
        ;mov eax , 1
        ;call read_sectors

        mov al , [0x1BE +si + 0x7C00]

@@:
        cmp al , 0x80
        je PartitionFound

        add si , 16
        cmp si , 0x1FE
        jg @b

partitionNotFound:
        mov si  , PartitionNotFoundStr
        call display_string
        xor  ah,ah              ; Wait for keypress
        int  16h
        int  18h

PartitionFound:

        push si
        mov eax , [si+0x8+0x1BE+0x7C00]
        mov esi , eax
        mov  [BootedSector] , esi
        pop si

        xor eax , eax

        mov cl , [si+ 0x2+0x1BE+0x7C00]
        and cl , 111111b
        mov al , 15
        mov ah , 0x2

        mov bx , 0x7E00
        push si
        int 13h

        mov dl , [DriveNum]
        mov esi , [BootedSector]
        jmp 0x7E00



include 'services\conio.inc'
;include 'services\diskio.inc'

BootErrMsg db "Failed to boot",0
PartitionNotFoundStr db "Partition Not Found", 0
times 510 - ($ - $$) db 0x00
dw 0xAA55
