align 16
KERNEL_OBJECT_MAX_NUM equ 1024
KERNEL_OBJECT_TABLE_EL_SIZE equ 6
;KERNEL OBJECT
;dw - type
;dw - NumOfUsers
;dw - SegToKernelObj
;dw - AddrToKernelObject

NUM_OF_EXECUTORS equ 32
;KERNEL_OBJECT_EXECUTER
;dw - seg
;dw - addr
kernelObjectTableSeg dw 0x00
kernelObjectTableAddr dw 0x00
KernelObjectExecuters rd NUM_OF_EXECUTORS

kernel_objects_init:

    push eax 
    push es cx dx di

    mov eax , KERNEL_OBJECT_MAX_NUM *KERNEL_OBJECT_TABLE_EL_SIZE
    call malloc
    mov [kernelObjectTableAddr] , ax
    mov [kernelObjectTableSeg] , es


    mov di , ax
    mov al , 0
    mov cx , KERNEL_OBJECT_MAX_NUM *KERNEL_OBJECT_TABLE_EL_SIZE
    call memset 
    pop di dx cx es
    pop eax
    ret

FreeKernelObject:
    ret

;in
;ax - executer id
;bx - data seg
;cx - data addr
;out 
;carry
;ax - handle

AddKernelObject:
    
    push es bp bx cx dx si
    mov si , [kernelObjectTableAddr]
    mov bp , [kernelObjectTableSeg]
    mov es , bp
    mov dx , 0

@@:
    cmp word [es:si] , 0
    je .freeKobjFound
    add si , KERNEL_OBJECT_TABLE_EL_SIZE
    inc dx
    jmp @b
.freeKobjFound:


    mov [es:si] , ax
    add si , 2
    mov [es:si] , bx
    add si , 2
    mov [es:si] , cx

    mov ax , dx

    pop si dx cx bx bp es
    ret


;in 
;ax - executorID
;bx - executorSeg
;cx - executorAddr
;out 
;carry
;ax - error code
AddKernelObjectExecuter:

    push es si
    cmp ax , NUM_OF_EXECUTORS
    jge .error

    push bx
    mov bx , 4
    mul bx
    mov si , ax
    pop bx

    add si , KernelObjectExecuters
    mov [ds:si] , bx
    add si , 2
    mov [ds:si] , cx
    xor ax , ax
    jmp .end
.error:
    stc 
    mov ax , 0x1
.end:
    pop si es
    ret


currentExecutorAddr dw 0
currentExecutorSeg dw 0


;in
;ax - handle
KernelObjectExecute:    
    
    push ax bx dx si bp es

    xor dx , dx
    mov bx , KERNEL_OBJECT_TABLE_EL_SIZE
    mul bx

    mov si , [kernelObjectTableAddr]
    add si , ax
    mov bp , [kernelObjectTableSeg]
    mov es , bp

    mov di , [es:si]

    mov ax , di
    mov bx , 4
    mul bx

    mov di , ax
    add di , KernelObjectExecuters
    mov ax , [ds:di]
    mov [currentExecutorSeg] , ax
    add di , 2

    mov ax , [ds:di]
    mov [currentExecutorAddr] , ax

    call far dword [currentExecutorAddr]

    pop es bp si dx bx ax

    ret

;in
;ax - handle
;out
;carry
;ax - error code
FreeHandle:
    mov bx , 0
    call KernelObjectExecute
    ret