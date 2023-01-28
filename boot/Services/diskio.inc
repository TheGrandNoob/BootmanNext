;  edx:eax = starting sector in LBA format
;       cx = count of sectors to read
;       bx = physical address to store data
;       si = segment

read_sectors_ext:
        pushad

        push edx        ; offset 12
        push eax        ; offset 8

        push si         ; offset 4       segment
        push bx         ; offset 4       addr
           
        push cx         ; offset 2
        push 10h        ; offset 0
           
        mov  si,sp
        mov  ah,42h             ; read

        mov  dl,[DriveNum] ; dl = drive

        int  13h
           
        add  sp,16      ; remove the items from the stack

           
        popad                   ; restore eax, edx, ebx, and cx



        ret


; Input: ax - LBA value
; Output: ax - Sector
;         bx - Head
;         cx - Cylinder
LBACHS:
        push dx 
        xor dx,dx               
        mov bx, [nSecPerTrack]  
        div bx                  
        inc dx          
        push dx 

        xor dx,dx               
        mov bx, [nHeads]        
        div bx                  

        mov cx,ax               
        mov bx,dx               
        pop ax                  
        pop dx  
                
        ret
