
SEGMANT_ALLOC_TABLE_LO_SEG  dw 0
SEGMANT_ALLOC_TABLE_LO_SIZE  dw 0
SEGMANT_ALLOC_TABLE_LO_ADDR dw 0

SEGMANT_ALLOC_TABLE_HI_SEG  dw 0
SEGMANT_ALLOC_TABLE_HI_SIZE  dw 0
SEGMANT_ALLOC_TABLE_HI_ADDR dw 0

;EBDA_ADDR dq 0

;LOW_MEMORY_AMOUNT_KB dq 0
;HI_MEMORY_AMOUNT_KB dq 0
;HI_PROTECTED_MEMORY_AMOUNT_64KB dq 0

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

; OUT ES - allocated seg

memory_allocate_segment:
	
	ret

memory_init:
  	
	in al, 0x92
        or al, 2
        out 0x92, al

	call memory_detect
		
	mov bp , 0x1000
	mov es , bp
	
	mov ax , [LOW_MEMORY_AMOUNT_KB] 
	mov dl , 8*4

	mov si , 0
	mov [es:si] , al
	

	div dl
	
	ret


memset:
	ret