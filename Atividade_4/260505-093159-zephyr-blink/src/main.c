/* Define o endereço do registrador SIM_SCGC5 */
#define SIM_SCGC5 (*((volatile unsigned int*)0x40048038))
#define GPIOB_PDDR (*((volatile unsigned int*)0x400FF054))
#define GPIOB_PSOR (*((volatile unsigned int*)0x400FF044))
#define GPIOB_PDOR (*((volatile unsigned int*)0x400FF040))
#define PORTB_PCR19 (*((volatile unsigned int*)0x4004A04C))


void delayMs (int n) {
	volatile int i;
	volatile int j;
	for (i = 0; i < n; i++)
		for (j = 0; j < 21000; j++) {}
}

int main(void)
{   
    
    SIM_SCGC5 |= (1<<10);
    PORTB_PCR19 |= (1<<8);
    GPIOB_PDDR |= (1<<19);
    GPIOB_PDOR |= (0<<19);
    while (1){
        delayMs(500);
        GPIOB_PDOR |= (1<<19);
        delayMs(500);
        GPIOB_PDOR &= 0xFFF7FFFF;
    }



    return 0;
}