org 0x2000

use16
align 16
FORMAT BINARY AS 'SYS'

prg_begin:

jmp main

diskSubSysErrorStr db "Disk subsystem error$"

ConfigPathStr db "/System16/config.cfg$"
AutorunPathStr db "/System16/config.cfg$"

        include 'proc16.inc'
        include 'commondata.inc'
        include 'int21h.inc'
        include 'disks.inc'
        include 'shell.inc'
        include 'process.inc'
        include 'memory.inc'
        include 'string.inc'
        include 'krnlobject.inc'
        include 'errorcodes.inc'
        include "iso9660.inc"
        include "FatFs.inc"
        include "NullFs.inc"
main:

        call disks_init_varables

        call int21h_init
	call memory_init  
        call kernel_objects_init
              
        call disks_init
        jc .diskError


        call init_process_manager

        ccall attach_fs_executor ,NullFs_executor 
        ccall attach_fs_executor ,isofs_executor
        ccall attach_fs_executor ,fatfs_executor

        mov ax , 0

        ccall attach_partition, 32 , edx , 0

        
        ;ccall CreateFile, AutorunPathStr ,FILE_OPEN_EXISTING
        ;ccall ReadFile , eax , cs ,dword DISK_RW_BUFFER , dword 4096
        ;ccall FreeHandle , eax

        call shell_task
        call poweroff

.diskError:
        mov dx , diskSubSysErrorStr
        jmp .error
.error:

        int 21h
        mov ah , 0
        int 16h


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
align 16
DISK_RW_BUFFER:
rb 4096

prg_end: