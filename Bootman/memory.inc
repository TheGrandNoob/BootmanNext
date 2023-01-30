SEGMENT_SIZE equ 0x1000


SEGMENT_ALLOC_TABLE_LO_SEG  dw 0
SEGMENT_ALLOC_TABLE_LO_SIZE  dw 0
SEGMENT_ALLOC_TABLE_LO_ADDR dw 0

SEGMENT_ALLOC_TABLE_HI_SEG  dw 0
SEGMENT_ALLOC_TABLE_HI_SIZE  dw 0
SEGMENT_ALLOC_TABLE_HI_ADDR dw 0

memory_init:
  	
	in al, 0x92
    or al, 2
	out 0x92, al

	call memory_detect
		
	mov bp , SEGMENT_SIZE
	mov es , bp

	mov si , 0	

	mov bx , 8*4
	mov ax , 0
	mov dx , 1
	div bx
	add ax , 3
	mov [es:si] , ax		;STORE Segment entry table size
	add si , 2
	mov [es:si] , byte 0x00
	

	;mov ax , [LOW_MEMORY_AMOUNT_KB] 

	mov [SEGMENT_ALLOC_TABLE_LO_SEG] , es
	mov si ,  [es:si]
	mov [SEGMENT_ALLOC_TABLE_LO_ADDR] , si


	mov bx , 0
	call memory_set_segment_status_used

	mov bx , SEGMENT_SIZE
	call memory_set_segment_status_used

	mov bx , SEGMENT_SIZE
	call memory_set_segment_status_used

	call memory_search_free_seg
	test ax , ax


	ret

;in
;ax - size

;out 
;es - seg
;ax - addr

malloc:

	ret
memory_detect:

    xor cx , cx
	xor dx , dx
	mov ax , 0xE801
	int 15h
	jc @f
	cmp ah , 0x86
	je @f
	cmp ah , 0x80
	je @f

	mov [HI_MEMORY_AMOUNT_KB] , cx
	mov [HI_PROTECTED_MEMORY_AMOUNT_64KB], dx
	 
@@:
	clc
	mov ah , 0x88
	int 15h
	jc @f
	test ax , ax
	je @f
@@:

	clc
	int 12h
	
	mov [LOW_MEMORY_AMOUNT_KB] , ax
	ret

;out 
;es - segment
;ax - address
allocate_block:

	ret
; OUT ES - allocated seg

memory_allocate_segment:
	
	ret

;in
;bx - segment

;out 
;carry 
;ax - error code if carry 
memory_set_segment_status_free:

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

	mov ax , bx
	mov bx , SEGMENT_SIZE
	
	div bx
	mov bl , 8
	div bl

	xchg bx , ax

	xor ax , ax
	mov al , bl
	add si , ax

	mov al , 1

	mov cl , bh
	sal al , cl

	mov bl , [es:si]

	not al
	and bl , al
	mov [es:si] , bl

	ret

	memory_set_segment_status_used:

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

	mov ax , bx
	mov bx , SEGMENT_SIZE
	
	div bx
	mov bl , 8
	div bl

	xchg bx , ax

	xor ax , ax
	mov al , bl
	add si , ax

	mov al , 1

	mov cl , bh
	sal al , cl

	mov bl , [es:si]
	or bl , al
	mov [es:si] , bl

	ret


;out
;es - free segment

memory_search_free_seg:
	
	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]
	mov bx , [SEGMENT_ALLOC_TABLE_LO_SIZE]
	add bx , si

.lo_searchLoop:
	mov al , [es:si]
	mov ah , al

	and al , 00000001b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end


@@:
	mov al , ah
	and al , 00000010b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 1

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 00000100b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 2

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 00001000b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 3

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 00010000b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 4

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 00100000b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 5

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 01000000b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 6

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:
	mov al , ah
	and al , 10000000b
	test al , al
	jnz @f

	mov al , ah

	mov bx , si
	sub bx , word [es:0]
	xchg bx , ax
	mov dx , 8
	mul dx
	
	add ax , 7

	mov dx , SEGMENT_SIZE
	mul dx

	jmp .end

	jnz @f
@@:

	inc si
	cmp si , bx
	jne .lo_searchLoop

.end:
	mov bp , ax 
	mov es , bp

	ret


;in 
;al - val
;cx - size
;di - dst address
;es - dst seg
memset:

	rep stosb

	ret


;in 
;al - val
;cx - size
;di - src address
;es - src seg
;di - dst address
;es - dst seg
memcpy:

	rep movsb

	ret