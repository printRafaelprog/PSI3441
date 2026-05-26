#include "MKL25Z4.h"


int main(void)
{   
    SIM->SCGC5 |= (1<<10);
    PORTB->PCR[19] |= (1<<8);
    SIM->SCGC6 |= (1<<27);
    ADC0->SC2 &= 0xFFFFFFBF;
    
    

    return 0;
}