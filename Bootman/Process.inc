align 16
attach_kernel_process:
    ret
init_process_manager:

    push ax bx cx

    mov ax , KERNEL_OBJECT_TYPE_PROCESS
    mov bp , cs
    mov bx , bp
    mov cx , process_kernelObjectExecutor
    call AddKernelObjectExecuter

    pop cx bx ax
    ret

;in 
;type stdcall
;arg1 - path
;arg2 - argc
;arg3 - argv
;out
;ax - handle
CreateProcess:

    push  bx cx

    mov bx , 0x200
    mov ax , KERNEL_OBJECT_TYPE_PROCESS
    mov cx , process_kernelObjectExecutor
    call AddKernelObject

    pop cx bx 
    ret


KOBjExecutorStr_Process db "called process executor",0x0A,0x0D,'$'
process_kernelObjectExecutor:
    mov dx , KOBjExecutorStr_Process
    mov ah , 0x9
    int 21h
    retf

;arg1 - path
exec:

    
    ccall CreateFile, [bP+2] ,FILE_OPEN_EXISTING
    mov edx , eax
    call memory_allocate_segment
    mov eax , 0
    mov ax , es
    ccall ReadFile , eax , eax , dword 65536 , dword BytesRead

    mov dx , [bp+2]
    ret