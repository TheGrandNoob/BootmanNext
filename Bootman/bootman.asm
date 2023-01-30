use16
align 16
FORMAT BINARY AS 'SYS'

prg_begin:

jmp main


HelloWorldStr db "HelloWorld$"

main:

        call disks_init
        call int21h_init
	call memory_init        

        call shell_task
        call poweroff


poweroff:

        mov     ax, 5301h
        xor     bx, bx
        int     15h

        ;try to set APM version (to 1.2)
        mov     ax, 530Eh
        mov     cx, 0102h
        xor     bx, bx
        int     15h

        ;turn off the system
        mov     ax, 5307h
        mov     bx, 0001h
        mov     cx, 0003h
        int     15h
        hlt
        ret

include 'commondata.inc'
include 'int21h.inc'
include 'disks.inc'
include 'shell.inc'
include 'memory.inc'
align 16
DISK_RW_BUFFER:
rb 4096

prg_end: