#include "MKL25Z4.h"
#include <stdio.h>


int main(void)
{   
    //ADC
    //liga o clock pra porta B
    SIM->SCGC5 |= (1<<10);
    // configuraos o mux do pino A0 pra servir de analog
    PORTB->PCR[0] |= ~(0b111<<8);
    // liga o colck pra porta do ADC
    SIM->SCGC6 |= (1<<27);
    
    //ADC CFG1 e o SC2 javem configurados da forma correta

    // setando canal 8, que esta ligado ao pino A0 
    ADC0->SC1[0] =  ADC_SC1_ADCH(8);
    
    //setando led azul
    //setando clock da porta D (led azul)
    SIM->SCGC5 |= (1<<12);
    //setando  led azul como GPIO
    PORTD->PCR[1] |= (1<<8);
    //setando o led azul como output
    GPIOD->PDDR |= (1<<1);

    //setando led verde
    //clock do led verde e o clock da porta B que ja foi setado
    //setando  led verde como GPIO
    PORTB->PCR[19] |= (1<<8);
    //setando o led verde como output
    GPIOB->PDDR |= (1<<19);
    uint16_t valor = ADC0->R[0];
    
    while(1){
        if (NXP_FLD2VAL(ADC_SC1_COCO, ADC0->SC1[0]) == 1){
            valor = ADC0->R[0];
            printk("%u\n", valor); //printa o valor lido
            if (valor < 100){
                
                // desliga o led azul quando 0
                GPIOD->PDOR |= (1<<1);
                // liga do led verde quando 0
                GPIOB->PDOR &= ~(1<<19);
            }
            else{
                // liga do led azul quando 3.3
                GPIOD->PDOR &= ~(1<<1);
                //desliga o led verde 
                GPIOB->PDOR |= (1<<19);
            }
        }
        // redefine o canal para iniciara conversao de novo
        ADC0->SC1[0] = ADC_SC1_ADCH(8);
    }

    
    

    return 0;
}