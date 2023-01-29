use16
align 16
FORMAT BINARY AS 'SYS'

jmp main


HelloWorldStr db "HelloWorld$"

main:

        call disks_init
        call int21h_init

        in al, 0x92
        or al, 2
        out 0x92, al
        
        mov ah , 9
        mov dx , HelloWorldStr
        int 21h
        jmp $



bootmanLoop:

        jmp bootmanLoop

include 'commondata.inc'
include 'int21h.inc'
include 'disks.inc'

align 16
DISK_RW_BUFFER: