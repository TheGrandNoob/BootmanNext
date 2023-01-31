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
	mov [es:si] , ax		;STORE Segment entry table size
	add si , 2
	mov [es:si] , byte 0x00
	sub si , 2

	mov [SEGMENT_ALLOC_TABLE_LO_SEG] , es
	mov si ,  [es:si]
	mov [SEGMENT_ALLOC_TABLE_LO_ADDR] , si

	mov ax , [LOW_MEMORY_AMOUNT_KB]
	mov cx ,  1024
	mul cx

	mov cx ,  SEGMENT_SIZE*10
	div cx


	cmp dx , 0
	je @f
	inc ax
	mov dx , 0
@@:

	mov cx , 8
	div cx

	cmp dx , 0
	je @f
	inc ax
@@:

	mov [SEGMENT_ALLOC_TABLE_LO_SIZE] , ax

	mov al , 0
	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	mov di , [SEGMENT_ALLOC_TABLE_LO_ADDR]
	mov cx , [SEGMENT_ALLOC_TABLE_LO_SIZE]
	call memset


	mov es , bp
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

	mov ax , 1
	mov [es:si] , ax
	mov bx , 0
@@:
	call memory_search_free_seg
	
	mov bp , es
	mov bx , bp

	call memory_set_segment_status_used

	cmp bx , 0
	jne @b

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

	push bp es si bx cx

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

	pop cx bx si es bp

	ret
;in
;bx - segment

;out 
;carry - error

memory_set_segment_status_used:

	push bp es si bx cx

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

	pop cx bx si es bp

	ret

;in 
;bx - start from
;

;out
;es - free segment

memory_search_free_seg:
	
	push bp si di ax bx cx dx

	xchg ax , bx

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]
	;mov bx , [SEGMENT_ALLOC_TABLE_LO_SIZE]
	;add bx , si

	mov cx , SEGMENT_SIZE
	div cx
	mov bx , ax
	mov cx , 0x7
	div cx

	add si , ax
	mov di , dx

	mov dx , bx
	mov bx , [SEGMENT_ALLOC_TABLE_LO_SIZE]

.lo_searchLoop:

	.lo_searchloop_inByte:

		mov ax , 0
		cmp dx , bx
		jg .end

		mov ah , [es:si]

	
		mov al , 1
		mov cx , di
		sal al , cl

		and ah , al
		cmp ah ,  0

		jne @f

		mov ax , si
		sub ax , [SEGMENT_ALLOC_TABLE_LO_ADDR]
		
		mov dx , 8
		mul dx

		add ax , di
		mov dx , SEGMENT_SIZE
		mul dx

		jmp .end

	@@:
		inc di
		inc dx
		cmp di , 8
	jne .lo_searchloop_inByte

	inc si
	
	jmp .lo_searchLoop
.end:

	mov bp , ax 
	mov es , bp

	pop dx cx bx ax di si bp

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