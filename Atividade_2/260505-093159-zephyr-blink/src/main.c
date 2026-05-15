#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <zephyr/drivers/uart.h>

#include <stdio.h>
#include <stdlib.h>

// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_50  = TPM_MODULE/2;       // 50% de duty cycle (meio brilho)

// Get the UART device binding
const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));


int main(void)
{    
    if (!device_is_ready(uart_dev)) {
        return 1;
    }

    char rx_buf[16];
    int buf_ptr = 0;

    // Inicializa o módulo TPM2 com:
    // - base do TPMx
    // - fonte de clock PLL/FLL (TPM_CLK)
    // - valor do registrador MOD
    // - tipo de clock (TPM_CLK)
    // - prescaler de 1 a 128 (PS)
    // - modo de operação EDGE_PWM
    float porcentagem = 0.0;
    float divisor_R = 1 + 1*porcentagem;
    float divisor_G = 1 + 1*porcentagem/2;
    uint16_t duty_50_R  = TPM_MODULE/divisor_R;
    uint16_t duty_50_G  = TPM_MODULE/divisor_G;
    pwm_tpm_Init(TPM2, TPM_OSCERCLK, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Inicializa o canal 0 do TPM2 para gerar sinal PWM na porta GPIOB_18
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);

    // Define o valor do duty cycle: nesse caso, duty_100 (LED quase desligado)
    

    // Loop infinito
    while (true){
        uint8_t c;
        if (uart_poll_in(uart_dev, &c) == 0){
            if(c == '\r'){
                continue;
            }
            uart_poll_out(uart_dev, c);
            if(c == '\n' || c == '\r'){

                rx_buf[buf_ptr] = '\0';
                if(buf_ptr>0){
                    int porcentagem = atoi(rx_buf);
                    if(porcentagem < 0) porcentagem = 0;
                    if (porcentagem > 100) porcentagem = 100;
                    divisor_R = 1 + 1*porcentagem/100;
                    divisor_G = 1 + 1*porcentagem/200;
                    duty_50_R = TPM_MODULE/divisor_R;
                    duty_50_G = TPM_MODULE/divisor_G;
                    printk("rodei");
                    pwm_tpm_CnV(TPM2, 0, duty_50_R);
                    pwm_tpm_CnV(TPM2, 1, duty_50_G);

                    
                }
                buf_ptr = 0;
                memset(rx_buf, 0, sizeof(rx_buf));
                
                        
                    }
                
            
            else{
                    if((c >= '0') && (c <= '9')){
                        if(buf_ptr < sizeof(rx_buf)-1){
                            rx_buf[buf_ptr++] = c;
                        }
                    }   
                 }
            }
    
    }
    return 0;
}