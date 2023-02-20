typedef struct gdt
{
    char common;
    unsigned char acces;
    unsigned char baseHi;
    unsigned int base;
    unsigned int limit;
}gdt;

void loadGDT();

int addSegment(unsigned long base , unsigned long size , unsigned char accesByte);


