#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 500

// Define o LED usando Device Tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

// Verifica se o LED está definido no Device Tree
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
static const struct gpio_dt_spec led_0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
static const struct gpio_dt_spec led_1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#else
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec led_2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

int transicao_estados(int* estado){
	if (*estado == 0){
        *estado = 1;
		return 1;
	}
	if (*estado == 1){
        *estado = 2;
		return 2;
	}
	if (*estado == 2){
        *estado = 0;
		return 0;
	}
}

void fluxo_dados(int* estado){
	if (*estado == 0){
		gpio_pin_set_dt(&led_2, 0);
        gpio_pin_set_dt(&led_0, 1);
	}
	if (*estado == 1){
        gpio_pin_set_dt(&led_0, 0);
		gpio_pin_set_dt(&led_1, 1);
	}
	if (*estado == 2){
        gpio_pin_set_dt(&led_1, 0);
		gpio_pin_set_dt(&led_2, 1);
	}
}

void main(void)
{
    int ret;
    int estado = 0;
	int* ptestado = &estado; 
	int estado_atual = 0;
    // Verifica se o device está pronto
    if (!gpio_is_ready_dt(&led_0)) {
        printk("Error: LED device %s is not ready\n", led_0.port->name);
        return;
    }

    // Configura o pino como saída
    ret = gpio_pin_configure_dt(&led_0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

    printk("LED blinking on %s pin %d\n", led_0.port->name, led_0.pin);

	// Verifica se o device está pronto
    if (!gpio_is_ready_dt(&led_1)) {
        printk("Error: LED device %s is not ready\n", led_1.port->name);
        return;
    }

    // Configura o pino como saída
    ret = gpio_pin_configure_dt(&led_1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

    printk("LED blinking on %s pin %d\n", led_0.port->name, led_1.pin);

	    // Verifica se o device está pronto
    if (!gpio_is_ready_dt(&led_2)) {
        printk("Error: LED device %s is not ready\n", led_2.port->name);
        return;
    }

    // Configura o pino como saída
    ret = gpio_pin_configure_dt(&led_2, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin\n", ret);
        return;
    }

    printk("LED blinking on %s pin %d\n", led_2.port->name, led_2.pin);

    while (1) {
        // Toggle do LED usando a nova API
        fluxo_dados(ptestado);
		estado_atual = transicao_estados(ptestado);
        k_msleep(SLEEP_TIME_MS);

    }
}