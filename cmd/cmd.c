
char* helloCmdStr = "HelloWorld$";

void putstr(char* str){
    asm("mov dx , [bp+2]\n"
        "mov ah , 0x9\n"
        "int 21h \n");
}

int _start(){

    putstr(helloCmdStr);
    while(1);
}