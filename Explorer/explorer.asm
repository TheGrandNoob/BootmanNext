format MZ                       ;Исполняемый файл DOS EXE (MZ EXE)
entry code_seg:start            ;Точка входа в программу
stack 200h                      ;Размер стека 
;--------------------------------------------------------------------
segment data_seg                ;Cегмент данных
    hello db 'Hello, asmworld!$'    ;Строка 
;--------------------------------------------------------------------
segment code_seg                ;Сегмент кода
start:                          ;Точка входа в программу
    mov ax,data_seg             ;Инициализация регистра DS
    mov ds,ax 
 
    mov ah,09h 
    mov dx,hello             
    int 21h 
 

    jmp $
    mov ax,4C00h
    int 21h                    