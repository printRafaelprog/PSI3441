#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 1024
#define PRIORITY_PADEIRO 5
#define PRIORITY_CLIENTE 2

volatile int saldo_vitrine = 0;

K_MUTEX_DEFINE(vitrine_mutex);

void padeiro(void *arg1, void *arg2, void *arg3) {
    while(1){
        k_msleep(1500);
        k_mutex_lock(&vitrine_mutex, K_FOREVER);
        saldo_vitrine++;
        printk("O saldo da vitrine e: %d\n", saldo_vitrine);
        k_mutex_unlock(&vitrine_mutex);
    }
     
}

K_THREAD_DEFINE(padeiro_thread, STACK_SIZE, padeiro,
                NULL, NULL, NULL,
                PRIORITY_PADEIRO, 0, 0);


void cliente(void *arg1, void *arg2, void *arg3) {
    while(1){
        k_msleep(1000);
        k_mutex_lock(&vitrine_mutex, K_FOREVER);
        saldo_vitrine--;
        printk("O saldo da vitrine e: %d\n", saldo_vitrine);
        k_mutex_unlock(&vitrine_mutex);
    }

}


K_THREAD_DEFINE(cliente_thread, STACK_SIZE, cliente,
                NULL, NULL, NULL,
                PRIORITY_CLIENTE, 0, 0);

int main(void)
{
  return 0;
}


