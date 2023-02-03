BootmanSeg dw 0x00
MainDiskID db 0x00

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