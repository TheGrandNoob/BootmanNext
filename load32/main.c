int main();

void _start(){
    main();
}

#include <gdt.h>

char* helloCmdStr = "HelloWorld$";

void putstr(char* str){
    asm("mov dx , [bp+2]\n"
        "mov ah , 0x9\n"
        "int 21h \n");
}

int main(){
    putstr(helloCmdStr);
    putstr(helloCmdStr);
    while(1);
}

