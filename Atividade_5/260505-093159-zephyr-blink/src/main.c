#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <zephyr/drivers/uart.h>

#include <stdio.h>
#include <stdlib.h>

//pretendo gera um PWM de 25kHz de frequencia e duty cicle de 33%
// para isso devo usar TPM_MODULE de 1000 TPM_CLK 48MHz (definido automatico pelo zephyr)
// E um Prescaler de 2, que seria um PS_2


// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_4  = TPM_MODULE/3;       // 25% de duty cycle


#define TPM_IRQ_LINE TPM1_IRQn  // relaciona a interrupção ao timer TPM1
#define TPM_IRQ_PRIORITY 1      // define a prioridade da interrupção

volatile uint16_t captured= 0; 
volatile uint16_t capturedold= 0; 
volatile uint16_t dif= 0; 
void tpm1_isr(void *arg)
{
       TPM1->STATUS |= TPM_STATUS_CH0F_MASK; // zerra a flag que gerou a interrupção
       capturedold = captured;
       captured = TPM1->CONTROLS[0].CnV; // coloca o valor atual do timer na variável "captured"
       dif = captured - capturedold;
}

int main(void)
{   
    // Conecta a interrupção via Zephyr
    IRQ_CONNECT(TPM_IRQ_LINE, TPM_IRQ_PRIORITY, tpm1_isr, NULL, 0);
    irq_enable(TPM_IRQ_LINE);

    // Inicializa TPM1 com módulo e prescaler desejado
    /*
    a linha abaixo apesar de parecerque estamos configurando um PWM, so utilizamos para inicializar o timer TMP1
    esse timer conta com o clock defnido por TMP_PLLFLL (48MHz) e conta ate 65535. Entao ele tem resulucao de 
    0.02us e conta até 1.37ms
    */
    pwm_tpm_Init(TPM1, TPM_PLLFLL, 65535, TPM_CLK, PS_1, EDGE_PWM); // ele conta com a frequecia do core ja que o pre escalor e 1
    // Configura TPM1_CH0 como input capture na borda de subida
    pwm_tpm_Ch_Init(TPM1, 0, TPM_INPUT_CAPTURE_BOTH| TPM_CHANNEL_INTERRUPT, GPIOB, 0);

    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_2, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 3);
    pwm_tpm_CnV(TPM2, 1, duty_4);
    
    while (1)
    {
        printk("Valor do TPM1: %u\n", dif);
        k_msleep(1000);
    }
    

    return 0;
}