use16

GDT_LIMIT equ 65535

align 4

org 0x0
FORMAT BINARY AS 'SYS'


jmp main
include 'conio.inc'
TASK_NUM db 0
CURRENT_TASK db 0

saved_int8 dw 0
saved_int8_seg dw 0
disable_interputs:

        cli

        in eax , 0x70
        or eax , 0x80
        out 0x70 , eax
        ret

enable_interputs:

        in eax , 0x70
        and eax , 0x7F
        out 0x70 , eax
        ret

main:


        push ds
        push es

        xor ax , ax
        mov es , ax
        mov ax  , [es:0020h]
        mov [ds:saved_int8] , ax
        mov ax  , [es:0022h]
        mov [ds:saved_int8_seg] , ax
        pop es

        xor bx , bx
        mov ds , bx
        mov  dword  [ds:0020h],THREAD_MAN
        mov  [ds:0022h],cs
        pop     ds

        mov ax , simple_task_1
        call ATTACH_TASK
        jmp KERNEL

task1_str db "TASK_1:",0
simple_task_1:
        mov si , task1_str
        call display_string
        jmp $
task2_str db "TASK_2:",0
simple_task_2:
        mov si , task2_str
        call display_string
        jmp $
;in
;ax - task ptr
;bx - task segment

;out - al task id
;bx - error code
ATTACH_TASK:

        cli

        ;mov [TASK_NUM] , al
        ;mov [0x2000 +tasks + TASK_NUM+task_id] , al

        ;mov [0x2000 + tasks + TASK_DATA+1] , ah
        ;mov [0x2000 +tasks + TASK_DATA] , al

        push ax
        mov ax , 256
        mul [TASK_NUM]
        add ax , tasks

        mov bx , ax
        pop ax

        ;add dx , es
        ;push es

        ;mov es , dx

        ;pop dx

        mov al , [TASK_NUM]
        inc al
        sti
ret

kernel_str db 'kernel:',0
KERNEL:

kernel_loop:
        mov si , kernel_str
        call display_string
        mov ah , [0x2000 +tasks + TASK_DATA+1]
        mov al ,[0x2000 +tasks + TASK_DATA]
        call simple_task_2
kernel_end_task:
        mov al , [TASK_NUM]
        inc al
        mov [TASK_NUM] , al
        jge LAST_TASK
        jmp kernel_loop

LAST_TASK:
        mov al , 0
        mov [TASK_NUM] , al

        jmp kernel_loop



times 512 - ($-$$) db 0x00
thr_man_str db 'thread man:',0
THREAD_MAN:


        nop
        nop
        nop
        nop
        nop
        pop eax
        mov ax , 0x200
        push ax
        mov ax , kernel_end_task
        push ax
        mov     al,20H
        out     20H,al
        iret



org 0x0
tasks:

task:
        task_id db ?
        TASK_DATA:
                rb 255