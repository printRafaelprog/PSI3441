#include "ultrasom.h"

//pretendo gera um PWM de 375Hz de frequencia e duty cicle de 1%
// para isso devo usar TPM_MODULE de 1000 TPM_CLK 48MHz (definido automatico pelo zephyr)
// E um Prescaler de 128, que seria um PS_128

//isso garante um pulso de 26us


// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_1  = TPM_MODULE/100;       // 1% de duty cycle


#define TPM_IRQ_LINE TPM1_IRQn  // relaciona a interrupção ao timer TPM1
#define TPM_IRQ_PRIORITY 1      // define a prioridade da interrupção

volatile uint16_t captured= 0; 
volatile uint16_t capturedold= 0; 
volatile uint16_t dif= 0; 
void tpm1_isr(void *arg)
{
       TPM1->STATUS |= TPM_STATUS_CH0F_MASK; // zerra a flag que gerou a interrupção
       if (GPIOB->PDIR & (1 << 0)) {
       // PTB0 está em nível alto
           captured = TPM1->CONTROLS[0].CnV; // coloca o valor atual do timer na variável "captured"
       }
       else {
       // PTB0 está em nível baixo
           capturedold = captured;
           captured = TPM1->CONTROLS[0].CnV; // coloca o valor atual do timer na variável "captured"
           dif = captured - capturedold;
           capturedold = 0;
           captured = 0;
       }
       
}

float ultrasom_dist()
{   
    // Conecta a interrupção via Zephyr
    IRQ_CONNECT(TPM_IRQ_LINE, TPM_IRQ_PRIORITY, tpm1_isr, NULL, 0);
    irq_enable(TPM_IRQ_LINE);

    // Inicializa TPM1 com módulo e prescaler desejado
    /*
    a linha abaixo apesar de parecerque estamos configurando um PWM, so utilizamos para inicializar o timer TMP1
    esse timer conta com o clock defnido por TMP_PLLFLL (375kHz) e conta ate 65535. Entao ele tem resulucao de 
    2.6us e conta até 170ms

    para medir distancias com o sensor de ultra som, considerando que a velocidade do som é de 340m/s
    isso equivale a uma resolucao de 0.88mm e um alcance maximo de 58m.
    */
    pwm_tpm_Init(TPM1, TPM_PLLFLL, 65535, TPM_CLK, PS_128, EDGE_PWM);
    // Configura TPM1_CH0 como input capture na borda de subida
    pwm_tpm_Ch_Init(TPM1, 0, TPM_INPUT_CAPTURE_BOTH| TPM_CHANNEL_INTERRUPT, GPIOB, 0);

    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 3);
    pwm_tpm_CnV(TPM2, 1, duty_1);
    
    float dist = 0.0;

    dist = (dif * 0.88) / 2; // Calcula a distância com base no tempo medido (dif) e na velocidade do som
    k_msleep(100);
    
    return dist;
}