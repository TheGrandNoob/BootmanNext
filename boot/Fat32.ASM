org 0x00

RESVD_SECTS  equ  32
NUM_FATS equ 2
STACK_OFFSET  equ 4400h

jmp short start
nop

OEM_NAME     db  'MSWIN4.1'
nBytesPerSec dw  0200h         ; 512 bytes per Sector
nSecPerClust db  01            ; Sectors per Cluster
nSecRes      dw  RESVD_SECTS   ; Sectors reserved for Boot Record, info sect, etc.
nFats        db  NUM_FATS      ; Number of FATs
             dw  0             ; reserved
             dw  0             ; reserved
mDesc        db  0F0h          ; Medium Descriptor Byte
             dw  0             ; reserved
nSecPerTrack dw  18            ; Sectors per Track
nHeads       dw  02            ; Number of Heads
nSecHidden   dd  00h           ; Number of Hidden Sectors
nSecsExt     dd  2880          ; total number of sectors in volume
nSecPerFat   dd  23            ; sectors per fat
ExtFlags     dw  00000000b     ; flags
nFSVer       dw  00h           ; version 0.0
nRootClust   dd  02            ; Root Cluster number
nFSInfo      dw  01            ; sector number of the FSINFO struct (0 based)
nBKBootSect  dw  06            ; sector number of backup boot block (0 based)

times 12 db 0x0

DriveNum     db  00h           ; Physical drive number
nResByte     db  00h           ; Reserved
             db  29h           ; Signature for Extended Boot Record
SerNum       dd  12345678h     ; Volume Serial Number
VolName      db  'NO NAME    ' ; Volume Label
FSType       db  'FAT32   '    ; File system type

start:
        cli

        mov ax , 0x7C0
        mov ds , ax
        mov es , ax
        mov ss , ax
        mov sp , STACK_OFFSET

        sti

        jmp     0x7C0:T

T:                             
        mov [FsBeginSector] , 0
        mov [DriveNum] , dl

        mov ah , 0x8
        int 13h

        inc dh
        mov [DiskHeads] , dh
        mov al , cl
        and al , 111111b
        mov [DiskSectors] , al

        xor ax , ax
        and cl , 00000011b
        mov ah , cl
        mov al , ch
        inc ax
        mov [DiskCylinders] , ax

        xor eax , eax
        mov al ,  [nFats]
        mov ecx , [nSecPerFat]
        mul ecx

        mov esi , [FsBeginSector]
        add eax , esi
        xor esi , esi
        mov si , [nSecRes]
        add eax , esi
        mov [RootDirAddr] , eax

        mov bx, DiskRWBuffer
        mov si, 0x7C0
        mov cx ,10
        call read_sectors_ext

        ;
	;call LBACHS

	;mov ch , cl
	;mov dh , bl
	;mov cl , al
	;mov al , 2
	;mov bx , DiskRWBuffer: 
	;int 13h 

        mov si , bootman
        mov di , DiskRWBuffer

        xor cx , cx

        findSystemDirLoop:
        push si
        mov si , di
        call display_string
        pop si

        cmp [di] , ch
        je .NotFound

        pusha
        mov cl , 11
        rep cmpsb
        popa

        jz .Found

        add di , 32
        jmp  findSystemDirLoop
.Found:

        mov bx , DiskRWBuffer

        mov al , [di + 0x1A]
        mov ah , [di + 0x14]

        sub ax , 2
        mov esi , [RootDirAddr]
        add eax , esi

        mov bx, 0
        mov si, 0x200
        mov cx , 10
        call read_sectors_ext

        cli
        mov bp , 0x200
        mov es , bp
        mov ds , bp
        mov ss , bp
        mov sp , 0xFFFF
        sti

        jmp 0x200:000

.no_LBA:
        mov si , bootman
        call display_string
        xor ah , ah
        int 16h
        int 18h

.NotFound:
        mov si , NotFound
        call display_string
        xor ah , ah
        int 16h
        int 18h

include 'services\conio.inc'
include 'services\diskio.inc'

bootman db 0x42,0x4F,0x4F,0x54,0x4D,0x41,0x4E,0x20,0x53,0x59,0x53,0x20,0x0 ;'BOOTMAN SYS',0
NotSupportedStr db 'Not supported',0
NotFound db 'bootman.sys not found' , 0
FsBeginSector dd 0x00

RootDirAddr dd 0x00
DiskCylinders dw 0x00
DiskHeads db 0x00
DiskSectors db 0x00



times 510 - ($ - $$) db 0x00
dw 0xAA55

DiskRWBuffer:
