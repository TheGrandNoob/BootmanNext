;in
;arg1 - path
;arg2 - flags
;out
;eax - handle
CreateFile:

    mov bp , sp

    mov si , [bp+2]

    cmp [es:si+1] , byte ':'
    je .isGlobalPathWithDisk

.isGlobalPathWithDisk:  

    mov edi , FsPartitionTable
    add edi , 0;dword [currentPartition]
    mov esi , 0
    mov si , word [ds:edi + Partition.fsType]
    mov eax , 0
    mov ax , word [ds:edi + Partition.fsInfoAddr]
    mov edx , 0
    mov dx , word [ds:edi + Partition.fsInfoSeg]

    mov ebx , [bp+2]
    inc ebx
    push esi
    ccall fs_executor_exec, esi, FS_EXECUTOR_CMD_CREATE , ebx , eax , edx
    pop esi
    cmp eax , 0
    je .end

    mov ecx , esi

    mov si , [LastFileIndex]
    inc si 
    mov [LastFileIndex] , si
    dec si

    mov ax , si
    mov bx , FileSize
    mul bx 
    
    mov si , ax  
    mov [ds:si + FilesTableBegin + FileT.fsType] , cx
    mov ax , [LastTransactionDataAddr]
    mov [ds:si + FilesTableBegin + FileT.fsInfoAddr] , ax
    mov ax , [LastTransactionDataSeg]
    mov [ds:si + FilesTableBegin + FileT.fsInfoSeg] , ax

    mov esi ,  0
    mov si , [LastFileIndex]
    dec si
    mov eax , esi 
    ;mov ax , [ss:bp]

    ;mov ax , KERNEL_OBJECT_TYPE_FILE
    ;call AddKernelObject
.end:
    ret