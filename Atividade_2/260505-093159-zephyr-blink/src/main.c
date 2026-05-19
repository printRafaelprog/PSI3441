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

    char rx_buf[32];
    int buf_ptr = 0;
    float porcentagem = 0.0;
    uint16_t duty_50_R;
    uint16_t duty_50_G;

    pwm_tpm_Init(TPM2, TPM_OSCERCLK, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);

    

    // Loop infinito
    while (true){
        char c;
        while (uart_poll_in(uart_dev, &c) == 0){
            if(c == '\n' || c == '\r'){
                rx_buf[buf_ptr] = '\0';
                int porcentagem = atoi(rx_buf);
                if(porcentagem < 0) porcentagem = 0;
                if (porcentagem > 100) porcentagem = 100;
                duty_50_R = - TPM_MODULE*(porcentagem/100.0) + TPM_MODULE;
                duty_50_G = - TPM_MODULE*(porcentagem/100.0) + TPM_MODULE;
                pwm_tpm_CnV(TPM2, 0, duty_50_R);
                pwm_tpm_CnV(TPM2, 1, duty_50_G);
                buf_ptr = 0;
                }
            else{
                if(buf_ptr < sizeof(rx_buf) - 1) rx_buf[buf_ptr++] = c;
            }
        }               
    }
    return 0;
}