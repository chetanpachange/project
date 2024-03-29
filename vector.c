#define WEAK __attribute__ ((weak))

/* Symbols exported by linker script */
extern unsigned _etext, _data, _edata, _ebss, _stack;
#include<stdint.h>

void main(void);
void reset_handler(void);
void blocking_handler(void);
void null_handler(void);

void WEAK nmi_handler(void);
void WEAK hard_fault_handler(void);
void WEAK mem_manage_handler(void);
void WEAK bus_fault_handler(void);
void WEAK usage_fault_handler(void);
void WEAK sv_call_handler(void);
void WEAK debug_monitor_handler(void);
void WEAK pend_sv_handler(void);
void WEAK sys_tick_handler(void);
void WEAK wwdg_isr(void);
void WEAK pvd_isr(void);
void WEAK tamper_isr(void);
void WEAK rtc_isr(void);
void WEAK flash_isr(void);
void WEAK rcc_isr(void);
void WEAK exti0_isr(void);
void WEAK exti1_isr(void);
void WEAK exti2_isr(void);
void WEAK exti3_isr(void);
void WEAK exti4_isr(void);
void WEAK dma1_channel1_isr(void);
void WEAK dma1_channel2_isr(void);
void WEAK dma1_channel3_isr(void);
void WEAK dma1_channel4_isr(void);
void WEAK dma1_channel5_isr(void);
void WEAK dma1_channel6_isr(void);
void WEAK dma1_channel7_isr(void);
void WEAK adc1_2_isr(void);
void WEAK usb_hp_can_tx_isr(void);
void WEAK usb_lp_can_rx0_isr(void);
void WEAK can_rx1_isr(void);
void WEAK can_sce_isr(void);
void WEAK exti9_5_isr(void);
void WEAK tim1_brk_isr(void);
void WEAK tim1_up_isr(void);
void WEAK tim1_trg_com_isr(void);
void WEAK tim1_cc_isr(void);
void WEAK tim2_isr(void);
void WEAK tim3_isr(void);
void WEAK tim4_isr(void);
void WEAK i2c1_ev_isr(void);
void WEAK i2c1_er_isr(void);
void WEAK i2c2_ev_isr(void);
void WEAK i2c2_er_isr(void);
void WEAK spi1_isr(void);
void WEAK spi2_isr(void);
void WEAK usart1_isr(void);
void WEAK usart2_isr(void);
void WEAK usart3_isr(void);
void WEAK exti15_10_isr(void);
void WEAK rtc_alarm_isr(void);
void WEAK usb_wakeup_isr(void);
void WEAK tim8_brk_isr(void);
void WEAK tim8_up_isr(void);
void WEAK tim8_trg_com_isr(void);
void WEAK tim8_cc_isr(void);
void WEAK adc3_isr(void);
void WEAK fsmc_isr(void);
void WEAK sdio_isr(void);
void WEAK tim5_isr(void);
void WEAK spi3_isr(void);
void WEAK usart4_isr(void);
void WEAK usart5_isr(void);
void WEAK tim6_isr(void);
void WEAK tim7_isr(void);
void WEAK dma2_channel1_isr(void);
void WEAK dma2_channel2_isr(void);
void WEAK dma2_channel3_isr(void);
void WEAK dma2_channel4_5_isr(void);

__attribute__ ((section(".vectors")))
void (*const vector_table[]) (void) = {
	(void*)&_stack,
	reset_handler,
	nmi_handler,
	hard_fault_handler,
	mem_manage_handler,
	bus_fault_handler,
	usage_fault_handler,
	0, 0, 0, 0,		/* Reserved */
	sv_call_handler,
	debug_monitor_handler,
	0,			/* Reserved */
	pend_sv_handler,
	sys_tick_handler,
	wwdg_isr,
	pvd_isr,
	tamper_isr,
	rtc_isr,
	flash_isr,
	rcc_isr,
	exti0_isr,
	exti1_isr,
	exti2_isr,
	exti3_isr,
	exti4_isr,
	dma1_channel1_isr,
	dma1_channel2_isr,
	dma1_channel3_isr,
	dma1_channel4_isr,
	dma1_channel5_isr,
	dma1_channel6_isr,
	dma1_channel7_isr,
	adc1_2_isr,
	usb_hp_can_tx_isr,
	usb_lp_can_rx0_isr,
	can_rx1_isr,
	can_sce_isr,
	exti9_5_isr,
	tim1_brk_isr,
	tim1_up_isr,
	tim1_trg_com_isr,
	tim1_cc_isr,
	tim2_isr,
	tim3_isr,
	tim4_isr,
	i2c1_ev_isr,
	i2c1_er_isr,
	i2c2_ev_isr,
	i2c2_er_isr,
	spi1_isr,
	spi2_isr,
	usart1_isr,
	usart2_isr,
	usart3_isr,
	exti15_10_isr,
	rtc_alarm_isr,
	usb_wakeup_isr,
	tim8_brk_isr,
	tim8_up_isr,
	tim8_trg_com_isr,
	tim8_cc_isr,
	adc3_isr,
	fsmc_isr,
	sdio_isr,
	tim5_isr,
	spi3_isr,
	usart4_isr,
	usart5_isr,
	tim6_isr,
	tim7_isr,
	dma2_channel1_isr,
	dma2_channel2_isr,
	dma2_channel3_isr,
	dma2_channel4_5_isr,
};
	int i;
__attribute__ ((section(".handlerfunctions")))
void reset_handler(void)
{
	volatile unsigned *src, *dest;
	volatile uint32_t *CCR,*SHCSR;
	i=1;
	while(i);
	CCR=(uint32_t *)0xE000ED14;
	*CCR=528;
	SHCSR=(uint32_t *)0xE000ED24;
	*SHCSR=0x70000;

	asm("MSR msp, %0" : : "r"(&_stack));

	for (src = &_etext, dest = &_data; dest < &_edata; src++, dest++)
		*dest = *src;

//	while (dest < &_ebss)
//		*dest++ = 0;
	
	/* Call the application's entry point. */
	main();
}

	void **HARDFAULT_PSP;
	register void **stack_pointer asm("sp");
	unsigned int spt;
__attribute__ ((section(".handlerfunctions")))
void blocking_handler(void)
{
}

__attribute__ ((section(".handlerfunctions")))
void null_handler(void)
{
	/* Do nothing. */
}

__attribute__ ((section(".handlerfunctions")))
void hard_fault_handler(void)
{
	/* Do nothing. */
}
__attribute__ ((section(".handlerfunctions")))
void mem_manage_handler(void)
{
	extern unsigned _etext,end,_sumfunend,_sumfunstart;
	unsigned *src,*dest,*nxaddr,offset,*lraddr;
	int func_size;

	asm(
	"TST LR, #4;"
	"ITE EQ;"
	"MRSEQ R0, MSP;"
	"MRSNE R0, PSP;"
	);

	asm("mov %0, r0" : "=r"(lraddr));			
	nxaddr=lraddr[12];
	offset=((int)nxaddr)-0xc0000000;
		      
	func_size= &_sumfunend - &_sumfunstart;

		for(dest=&end,
			src=&_etext+(offset/2); src<= &_etext + (func_size/2);
				src++,dest++)
		{	
			*dest=*src;
		}	

      asm(
	"TST LR, #4;"
	"ITE EQ;"
	"MRSEQ R0, MSP;"
	"MRSNE R0, PSP;"
	);

	asm("mov %0, r0" : "=r"(lraddr));			
		lraddr[14] = &end;
		lraddr[12] = &end;
//		lraddr[2] = &end;				
	return;	

}

#pragma weak nmi_handler = null_handler
#pragma weak hard_fault_handler 
#pragma weak mem_manage_handler 
#pragma weak bus_fault_handler = blocking_handler
#pragma weak usage_fault_handler
#pragma weak sv_call_handler = null_handler
#pragma weak debug_monitor_handler = null_handler
#pragma weak pend_sv_handler = null_handler
#pragma weak sys_tick_handler = null_handler
#pragma weak wwdg_isr = null_handler
#pragma weak pvd_isr = null_handler
#pragma weak tamper_isr = null_handler
#pragma weak rtc_isr = null_handler
#pragma weak flash_isr = null_handler
#pragma weak rcc_isr = null_handler
#pragma weak exti0_isr = null_handler
#pragma weak exti1_isr = null_handler
#pragma weak exti2_isr = null_handler
#pragma weak exti3_isr = null_handler
#pragma weak exti4_isr = null_handler
#pragma weak dma1_channel1_isr = null_handler
#pragma weak dma1_channel2_isr = null_handler
#pragma weak dma1_channel3_isr = null_handler
#pragma weak dma1_channel4_isr = null_handler
#pragma weak dma1_channel5_isr = null_handler
#pragma weak dma1_channel6_isr = null_handler
#pragma weak dma1_channel7_isr = null_handler
#pragma weak adc1_2_isr = null_handler
#pragma weak usb_hp_can_tx_isr = null_handler
#pragma weak usb_lp_can_rx0_isr = null_handler
#pragma weak can_rx1_isr = null_handler
#pragma weak can_sce_isr = null_handler
#pragma weak exti9_5_isr = null_handler
#pragma weak tim1_brk_isr = null_handler
#pragma weak tim1_up_isr = null_handler
#pragma weak tim1_trg_com_isr = null_handler
#pragma weak tim1_cc_isr = null_handler
#pragma weak tim2_isr = null_handler
#pragma weak tim3_isr = null_handler
#pragma weak tim4_isr = null_handler
#pragma weak i2c1_ev_isr = null_handler
#pragma weak i2c1_er_isr = null_handler
#pragma weak i2c2_ev_isr = null_handler
#pragma weak i2c2_er_isr = null_handler
#pragma weak spi1_isr = null_handler
#pragma weak spi2_isr = null_handler
#pragma weak usart1_isr = null_handler
#pragma weak usart2_isr = null_handler
#pragma weak usart3_isr = null_handler
#pragma weak exti15_10_isr = null_handler
#pragma weak rtc_alarm_isr = null_handler
#pragma weak usb_wakeup_isr = null_handler
#pragma weak tim8_brk_isr = null_handler
#pragma weak tim8_up_isr = null_handler
#pragma weak tim8_trg_com_isr = null_handler
#pragma weak tim8_cc_isr = null_handler
#pragma weak adc3_isr = null_handler
#pragma weak fsmc_isr = null_handler
#pragma weak sdio_isr = null_handler
#pragma weak tim5_isr = null_handler
#pragma weak spi3_isr = null_handler
#pragma weak usart4_isr = null_handler
#pragma weak usart5_isr = null_handler
#pragma weak tim6_isr = null_handler
#pragma weak tim7_isr = null_handler
#pragma weak dma2_channel1_isr = null_handler
#pragma weak dma2_channel2_isr = null_handler
#pragma weak dma2_channel3_isr = null_handler
#pragma weak dma2_channel4_5_isr = null_handler
