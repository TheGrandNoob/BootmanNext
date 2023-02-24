org 0x2000

use16
align 16
FORMAT BINARY AS 'sys'

prg_begin:

jmp main

AutoRunFileHandle dd 0
BytesRead dd 0

diskSubSysErrorStr db "Disk subsystem error$"

ConfigPathStr db "\system16\config.cfg$"
AutorunPathStr db "\system16\autorun.cfg$"
CmdSysPath db "\system16\load32.sys$"

        include 'proc16.inc'
        include 'commondata.inc'
        include 'int21h.inc'
        include 'disks.inc'
        include 'process.inc'
        include 'memory.inc'
        include 'string.inc'
        include 'errorcodes.inc'
        include "iso9660.inc"
        include "FatFs.inc"
        include "NullFs.inc"
main:

        call disks_init_varables

        call int21h_init
	call memory_init  
              
        call disks_init
        jc .diskError


        call init_process_manager

        ccall attach_fs_executor ,NullFs_executor 
        ccall attach_fs_executor ,isofs_executor
        ccall attach_fs_executor ,fatfs_executor

        call memory_allocate_segment
        mov ax , 0
        mov es , ax

        mov edx , 0
        mov dl ,[MainDiskID]
        mov [BootDiskPartition] , 0xFFFF

        ccall attach_partition, [BootPartitionSector]  , edx , 0
        mov [currentPartition] , ax

        jc .diskError
        
        mov bp , 0
        mov es , bp

        ccall CreateFile, AutorunPathStr ,FILE_OPEN_EXISTING
        mov [AutoRunFileHandle] , eax

        ccall ReadFile , eax ,dword DISK_RW_BUFFER , dword 4096 , dword BytesRead
        ;ccall FreeHandle , [AutoRunFileHandle]

        ccall exec, DISK_RW_BUFFER

        mov ah , 0x9
        mov dx , DISK_RW_BUFFER
        int 21h

        mov edx , FileCfgErrorStr
        call .error

.diskError:
        mov edx , diskSubSysErrorStr
        jmp .error
.error:

        int 21h
        mov ah , 0
        int 16h

align 16
FileCfgErrorStr db "Cannot run:"
DISK_RW_BUFFER:
rb 4096

prg_end: