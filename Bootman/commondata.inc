BootmanSeg dw 0x00
MainDiskID db 0x00
currentPartition dw 0
BootPartitionSector dd 0x00
BootDiskPartition dw 0x00

struc partition{
    db DiskId
    db PartitionNum
    dw type
}

partitions rw 256
currentPath rb 256
partitionCount rb 0

cmdLine rb 256

ISO_Str db "CD001",0x00
NTFS_Str db "NTFS",0x00

FAT12_Str db "FAT12"
FAT16_Str db "FAT16"
FAT32_Str db "FAT32"


EBDA_ADDR dq 0

LOW_MEMORY_AMOUNT_KB dw 0
HI_MEMORY_AMOUNT_KB dw 0
HI_PROTECTED_MEMORY_AMOUNT_64KB dw 0

BPB_TYPE_FAT12 equ 1
BPB_TYPE_FAT16 equ 2
BPB_TYPE_FAT32 equ 3
BPB_TYPE_ISO equ 4
BPB_TYPE_MBR equ 5

PARTITON_FLAG_MOUTED equ 0x1
KRNL_DISK_IO_BUFFER rb 512*8

KERNEL_OBJECT_TYPE_NULL equ 0x00
KERNEL_OBJECT_TYPE_PROCESS equ 0x1
KERNEL_OBJECT_TYPE_FILE equ 0x2
KERNEL_OBJECT_TYPE_THREAD equ 0x3

FILE_CREATE_ALWAYS equ 0x4
FILE_NEW equ 0x1
FILE_OPEN_EXISTING equ 0x2

EXECUTOR_CMD_TYPE_READ_FILE equ 0x2

FS_EXECUTOR_CMD_INIT        equ 0x1
FS_EXECUTOR_CMD_FREE        equ 0x2
FS_EXECUTOR_CMD_VALIDATE    equ 0x3
FS_EXECUTOR_CMD_CREATE      equ 0x4
FS_EXECUTOR_CMD_ATTACH_PARTITION equ 0x5
FS_EXECUTOR_CMD_FREE_FILE equ 0x6
FS_EXECUTOR_CMD_READ_FILE equ 0x7

PTR_SIZE        equ 2

FILE_FLAG_DIR           equ 0x1
FILE_FLAG_EXECUTABLE    equ 0x2
FILE_FLAG_READABLE      equ 0x4
FILE_FLAG_WRITABLE      equ 0x8