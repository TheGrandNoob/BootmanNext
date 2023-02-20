all:
	smlrcc -flat32 main.c src/*.c -I include -o load32.sys
	cp load32.sys ../utils
clean:
	rm load32.sys