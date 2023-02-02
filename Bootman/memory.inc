SEGMENT_SIZE equ 0x1000


MEMORY_ALLOCATE_STRUCTURE_SIZE equ 0x10

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

	mov ax , 32 
	mov bp , 0x1000
	mov es , bp
	mov bx , 0x1000
	call memory_create_heap_segment

	mov si , bx 
	inc si

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

	mov cx , 64
	div cx

	dec ax

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
	call memory_set_segment_status_used

	mov ax , [SEGMENT_ALLOC_TABLE_LO_SIZE]
	call malloc

	push ds si

	mov cx , [SEGMENT_ALLOC_TABLE_LO_SIZE]
	mov di , ax

	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]
	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov ds , bp

	call memcpy

	pop si ds
	ret

;in
;ax - size
;out 
;es - seg
;ax - addr
malloc:

	push bx cx dx si di es

	mov bx , 0x1000
	call memory_search_free_seg	

	mov si , 2
	mov bx , [es:si]

	cmp bx , 0
	jne @f

	mov bx , 0x1000
	mov ax , 0x20
	call memory_create_heap_segment

	xor dx , dx

	mov si , 2
	mov bx , [es:si]
	mov cx , 0
@@:

	div bx

	cmp dx , 0
	je @f
	inc ax
@@:
	mov dx , ax

	mov ax , 0
	.search_free_chunk_of_blocks:
	mov si , dx

	dec si
	call memory_search_free_block

	cmp si , 0
	je .searchFineshed

	mov cx , ax
	mov dx , ax

	.searchInSegment:

		call memory_search_free_block

		add cx , bx
		cmp cx , ax
		jne .search_free_chunk_of_blocks

		dec si
		jmp .searchInSegment

	jmp .search_free_chunk_of_blocks

	.searchFineshed:


	mov si , 2
	mov bx , [es:si]
	mov si , dx

	push si

	mov cx , ax
	mov ax , bx
	
	mul si
	xchg ax , cx
	sub ax , cx

	pop si


	
	call memory_allocate_block
	dec si

	@@:
		cmp si , 0
		je @f

		call memory_allocate_block
		dec si

		jmp @b

	@@:

	pop es di si dx cx bx 

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



;in 
;ax - block size
;es - segment address
;out
;bx - end of heap structure
memory_create_heap_segment:

	mov cx , ax

	xor dx , dx
	mov bx , ax
	mov ax , (SEGMENT_SIZE*0x10-1)
	div cx

	xor dx , dx
	mov bx , 8
	div bx

	push ax bx cx di si

	add cx , ax
	mov al , 0
	mov di , 0
	call memset

	pop si di cx bx ax

	mov si , 0
	mov word [es:si] , ax

	add si , 2
	mov word [es:si] , cx

	add si , 2
	mov byte [es:si] , 1

	mov si , 0
	add si , MEMORY_ALLOCATE_STRUCTURE_SIZE

	mov bx , ax
	add bx , MEMORY_ALLOCATE_STRUCTURE_SIZE

	push bx

	mov di , 0

	.alloc_memalloc_struct_loop:
		mov di , 0
		.alloc_memalloc_struct_loop_byte:
		
		cmp bx , 0
		jle .end


		push cx

		mov cx , di
		mov al , 1
		sal al , cl
		
		mov ah , [es:si]
		or ah , al
		mov [es:si] , ah

		pop cx

		inc di
		sub bx , cx

		cmp di , 8
		jne .alloc_memalloc_struct_loop_byte
		inc si

	jmp .alloc_memalloc_struct_loop

.end:

	pop bx
	ret




;in
;es - segment
;ax - start from
;out
;ax - block address
memory_allocate_block:

	
	push bx cx dx si di 

	xor dx , dx
	
	mov si , 2
	mov bx , [es:si]
	add ax , bx
	div bx

	mov cx , ax
	
	xor dx , dx
	mov bx , 7
	div bx

	mov di , dx

	mov si , 0
	mov bx , [es:si]

	mov si , MEMORY_ALLOCATE_STRUCTURE_SIZE
	add si , ax
	mov di , dx
	
	mov dx , cx

	.lo_searchLoop:

	.lo_search_loop_byte:
		cmp dx , bx
		jge .end

		mov ah , [es:si]

		mov cx , di
		mov al , 1
		sal al , cl
		and ah , al

		cmp ah , 0
		jne @f

			mov ah , [es:si]
			mov cx , di
			mov al , 1
			sal al , cl 
			or ah , al
			mov [es:si] , ah

			mov ax , dx
			mov si , 2
			mov bx , [es:si]
			mul bx
			
			jmp .end

		@@:

		inc dx
		inc di
		cmp di , 8
		jnge .lo_search_loop_byte

	mov di , 0	
	inc si
	jmp .lo_searchLoop

.end:

	pop di si dx cx bx

	ret
;in
;es - segment
;ax - start from
;out
;ax - block address
memory_search_free_block:

	push bx cx dx si di 

	xor dx , dx
	
	mov si , 2
	mov bx , [es:si]
	add ax , bx
	div bx

	mov cx , ax
	
	xor dx , dx
	mov bx , 7
	div bx

	mov di , dx

	mov si , 0
	mov bx , [es:si]

	mov si , MEMORY_ALLOCATE_STRUCTURE_SIZE
	add si , ax
	mov di , dx
	
	mov dx , cx

	.lo_searchLoop:

	.lo_search_loop_byte:
		cmp dx , bx
		jge .end

		mov ah , [es:si]

		mov cx , di
		mov al , 1
		sal al , cl
		and ah , al

		cmp ah , 0
		jne @f

			mov ax , dx
			mov si , 2
			mov bx , [es:si]
			mul bx
			
			jmp .end

		@@:

		inc dx
		inc di
		cmp di , 8
		jnge .lo_search_loop_byte

	mov di , 0	
	inc si
	jmp .lo_searchLoop

.end:

	pop di si dx cx bx

	ret

; OUT ES - allocated seg
;out 
;es - segment
memory_allocate_segment:
	
	push bx bp
	call memory_search_free_seg
	mov bp , es
	mov bx , es
	call memory_set_segment_status_used

	pop bp bx
	ret

;in
;bx - segment
;out 
;carry 
;ax - error code if carry 
memory_set_segment_status_free:

	push bp es si bx cx

	xor dx , dx

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

	mov ax , bx 
	mov bx , SEGMENT_SIZE
	div bx

	mov bx , 7
	div bx

	add si , ax

	mov al , 1
	mov cl , dl
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

	xor dx , dx

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

	mov ax , bx 
	mov bx , SEGMENT_SIZE
	div bx

	mov bx , 7
	div bx

	add si , ax

	mov al , 1
	mov cl , dl
	sal al , cl

	mov ah , [es:si]
	or ah , al
	mov [es:si], ah

	pop cx bx si es bp

	ret

;in 
;bx - start from
;
;out
;es - free segment
memory_search_free_seg:
	
	push bp si di ax bx cx dx

	xor dx , dx
	xchg ax , bx

	mov bp , [SEGMENT_ALLOC_TABLE_LO_SEG]
	mov es , bp
	
	mov si , [SEGMENT_ALLOC_TABLE_LO_ADDR]

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
		jge .end

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
	push si ds
	rep stosb
	pop ds si
	ret

;in 
;cx - size
;si - src address
;ds - src seg
;di - dst address
;es - dst seg
memcpy:
	push si ds
	rep movsb
	pop ds si
	ret