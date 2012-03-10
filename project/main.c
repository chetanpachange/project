#include<stdint.h>
__attribute__((section("sumfunstart")))
//sumfunstart=.;
int sum()
{
return 2+1;
}
//sumfunend=.;
__attribute__((section("sumfunend")))

int main(void)
{
	extern unsigned sumfunstart,sumfunend;
	volatile unsigned *codeaddr,*src,*dest;
	int (*fp)(void);
	//asm volatile ("MOVS r0, #1"); 
	//asm volatile ("MSR PRIMASK, r0");
		extern int a;
	//_set_BASEPRI(0x0);
	codeaddr=(unsigned *)0x2000009a;
		for(dest=codeaddr,src=&sumfunstart;src<=&sumfunend;src++,dest++){
			*dest=*src;
		}	
        a=8;
	a=a/(a-8);
//	fp=codeaddr;
//	goto *codeaddr;
//	fp();	
	//a=5;
	sum();
	while(1);
}
int sub()
{
return 2-1;
}
void usage_fault_handler(){
	volatile uint32_t *CCR;	
	extern int a;
	a=25;
	//asm volatile("MOVS r0, 0xe000e2da");
	//asm volatile("MOVS [r0], #0");	
	CCR=(uint32_t *)0xe000ed14;
	*CCR=0x0;	

	//printf("HI");
	// volatile ("POP {lr}");
	//asm volatile ("MOVS pc,lr");
return;
}
/*void hard_fault_handler(){
	//extern int a;
	//a=25;	
	//pc=<>;	
	//asm volatile("MOVS pc, lr");
	//asm volatile("bx lr");
	//sum();
	//asm volatile ("MOVS r0, #0"); 
	//asm volatile ("MSR PRIMASK, r0");
	asm volatile ("POP {lr}");
	asm volatile ("MOVS pc,lr");	
	return;
}*/
