#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 1024
#define PRIORITY_PADEIRO 5
#define PRIORITY_CLIENTE 5

volatile int saldo_vitrine = 0;

K_SEM_DEFINE(produz, 10, 10);
K_SEM_DEFINE(consome, 0, 10);

void padeiro(void *arg1, void *arg2, void *arg3) {
    while(1){
        k_msleep(1500);
        k_sem_take(&produz, K_FOREVER);
        saldo_vitrine++;
        printk("O saldo da vitrine e: %d\n", saldo_vitrine);
        k_sem_give(&consome);
    }
}

K_THREAD_DEFINE(padeiro_thread, STACK_SIZE, padeiro,
                NULL, NULL, NULL,
                PRIORITY_PADEIRO, 0, 0);


void cliente(void *arg1, void *arg2, void *arg3) {
    while(1){
        k_msleep(1000);
        k_sem_take(&consome, K_FOREVER);
        saldo_vitrine--;
        printk("O saldo da vitrine e: %d\n", saldo_vitrine);
        k_sem_give(&produz);
    }
}


K_THREAD_DEFINE(cliente_thread, STACK_SIZE, cliente,
                NULL, NULL, NULL,
                PRIORITY_CLIENTE, 0, 0);

int main(void)
{
  return 0;
}


