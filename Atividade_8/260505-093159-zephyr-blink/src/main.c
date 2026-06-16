#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>


#define STACK_SIZE 1024
#define PRIORITY 5

//ADC
#define ADC_RESOLUTION      12
#define ADC_GAIN            ADC_GAIN_1
#define ADC_REFERENCE       ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID      8  //Canal do ADC, veja a pinagem
#define ADC_VREF_MV         3300
#define BUTTON_NODE DT_NODELABEL(user_button_0)

// === Endereço e registradores do MMA8451Q ===
#define MMA8451Q_I2C_ADDR    0x1D
#define MMA8451Q_CTRL_REG1   0x2A

// === Bits de configuração ===
#define MMA8451Q_ACTIVE_BIT  0x01
#define MMA8451Q_ODR   (0x0 << 3)  // 100 Hz conforme datasheet (DR=100b)


struct sensor_value accel_x, accel_y, accel_z;
int ret;

// === Obter dispositivos ===
static const struct device *const accel = DEVICE_DT_GET(DT_ALIAS(accel0));
static const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

uint32_t contador = 0;
uint32_t t0;

float accel_fx, accel_fy, accel_fz;

float accel_x[51] = {0};

float bs[51] = {

    0.0007,   -0.0000,   -0.0009,   -0.0016,   -0.0015,    0.0000,    0.0024,    0.0044,    0.0039,   -0.0000,   -0.0060,   -0.0103,   -0.0089,

    0.0000,    0.0128,    0.0217,    0.0184,   -0.0000,   -0.0268,   -0.0465,   -0.0411,    0.0000,    0.0727,    0.1570,    0.2244,    0.2502,

    0.2244,    0.1570,    0.0727,    0.0000,   -0.0411,   -0.0465,   -0.0268,   -0.0000,    0.0184,    0.0217,    0.0128,    0.0000,   -0.0089,

   -0.0103,   -0.0060,   -0.0000,    0.0039,    0.0044,    0.0024,    0.0000,   -0.0015,   -0.0016,   -0.0009,   -0.0000,    0.0007
}

float fir(float x){
    float x_filtrado = 0;
    for (int i = 0; i<51; ++i){
        x_filtrado = x_filtrado + bs[i]*x[i]
    }
    return x_filtrado;
}

void mma8451q_configurar_odr(void)
{
    uint8_t buf[2];
    int ret;

    // 1️⃣ Colocar o sensor em standby (necessário antes de mudar ODR)
    buf[0] = MMA8451Q_CTRL_REG1;
    buf[1] = 0x00;
    ret = i2c_write(i2c_dev, buf, 2, MMA8451Q_I2C_ADDR);
    if (ret) {
        printk("ERRO ao colocar MMA8451Q em standby (%d)\n", ret);
        return;
    }

    // 2️⃣ Configurar ODR = 100 Hz (bits DR[5:3] = 100)
    buf[0] = MMA8451Q_CTRL_REG1;
    buf[1] = MMA8451Q_ODR;
    ret = i2c_write(i2c_dev, buf, 2, MMA8451Q_I2C_ADDR);
    if (ret) {
        printk("ERRO ao configurar ODR (%d)\n", ret);
        return;
    }

    // 3️⃣ Ativar o sensor novamente
    buf[0] = MMA8451Q_CTRL_REG1;
    buf[1] = MMA8451Q_ODR | MMA8451Q_ACTIVE_BIT;
    ret = i2c_write(i2c_dev, buf, 2, MMA8451Q_I2C_ADDR);
    if (ret) {
        printk("ERRO ao ativar MMA8451Q (%d)\n", ret);
        return;
    }

    printk("MMA8451Q configurado para 100 Hz via I2C.\n");
}


K_MUTEX_DEFINE(print_mutex);
K_SEM_DEFINE(coleta, 0, 1);

void comunicacao(void *arg1, void *arg2, void *arg3) {
        // Formato: T: tempo_ms, X: valor, Y: valor, Z: valor
        while(1){
            k_sem_take(&coleta, K_FOREVER);
            k_mutex_lock(&print_mutex, K_FOREVER);
            contador++;

            uint32_t agora = k_uptime_get_32();

            if (agora - t0 >= 1000) {
                //printk("Taxa: %u prints/s\n", contador);

                contador = 0;
                t0 = agora;
            }
            accel_fx = fir(accel_x.val1 + abs(accel_x.val2)/1000000);
            accel_fy = fir(accel_y.val1 + abs(accel_y.val2)/1000000);
            accel_fz = fir(accel_z.val1 + abs(accel_z.val2)/1000000);
            printk("%d.%06d, %d.%06d, %d.%06d\r\n", 
               accel_fx,
               accel_fy,
               accel_fz);
            k_mutex_unlock(&print_mutex);
        }
        
}

K_THREAD_DEFINE(comunicacao_task, STACK_SIZE, comunicacao,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);


void accel_task(void *arg1, void *arg2, void *arg3) {


    k_mutex_lock(&print_mutex, K_FOREVER);
    // Verificar se o dispositivo está pronto
    if (!device_is_ready(accel)) {
        printk("ERRO: Acelerometro nao esta pronto!\n");
        return;
    }
    k_mutex_unlock(&print_mutex);
    while (1) {
        // Solicitar leitura do sensor
        k_mutex_lock(&print_mutex, K_FOREVER);
        ret = sensor_sample_fetch(accel);
        if (ret) {
            printk("Erro ao ler sensor: %d\n", ret);
            k_sleep(K_MSEC(500));
            continue;
        }

        // Obter valores dos eixos X, Y e Z
        sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &accel_x);
        sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &accel_y);
        sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &accel_z);


        k_mutex_unlock(&print_mutex);
        k_sem_give(&coleta);
        // Aguardar 1000ms antes da próxima leitura
    }
}


K_THREAD_DEFINE(thread_accel, STACK_SIZE, accel_task,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);


int main(void)
{
  t0 = k_uptime_get_32();
  mma8451q_configurar_odr();
  return 0;
}


