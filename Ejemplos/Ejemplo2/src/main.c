/*
 * Resumen:
 * Este código muestra cómo usar FreeRTOS para crear dos 
 * tareas con prioridades diferentes.
 * Documentacion: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
 *				  https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para la UART0
 *
 * Fecha: 23/05/2024
 * Autores: Espindola Agustin, Glas Sebastian
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

// Definir el núcleo a utilizar
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Definiciones de la UART
#define UART_PORT_NUM      UART_NUM_0
#define UART_TX_PIN        (GPIO_NUM_1)
#define UART_RX_PIN        (GPIO_NUM_3)
#define UART_BAUD_RATE     (300)
#define UART_TX_BUF_SIZE   (512)
#define UART_RX_BUF_SIZE   (512)

// Un string para enviar por el puerto
const char msg[] = "tecnicas digitales en accion procesando en el nucleo 0";

// Handles para las tareas
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

// Tareas
// Task1 imprime en el terminal serial con baja prioridad
void startTask1(void *parameter)
{
    // Contar el número de caracteres en el string
    int msg_len = strlen(msg);

    char jump [] = "\n";

    // Imprimir el string en el terminal serial
    while(1)
    {
        uart_write_bytes(UART_PORT_NUM, jump, 1);
        for(int i=0; i<msg_len; i++)
        {
            uart_write_bytes(UART_PORT_NUM, &msg[i], 1);
            vTaskDelay(10/ portTICK_PERIOD_MS);
        }
        uart_write_bytes(UART_PORT_NUM, jump, 1);
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

// Task2 imprime en el terminal serial con prioridad superior
void startTask2 (void *parameter)
{
    char text [] = "x";
    while(1)
    {
        uart_write_bytes(UART_PORT_NUM, text, 1);
        vTaskDelay(100/ portTICK_PERIOD_MS);
    }
}

void app_main() 
{
    // Configuración de la UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // Configurar el número de puerto UART
    uart_param_config(UART_PORT_NUM, &uart_config);

    // Configurar los pines para TX y RX
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Instalar el controlador UART
    uart_driver_install(UART_PORT_NUM, UART_RX_BUF_SIZE * 2, UART_TX_BUF_SIZE * 2, 0, NULL, 0);

    // Esperar un momento
    printf("\n---Demostración de Tareas FreeRTOS---\n");

    // Imprimir prioridad propia
    printf("Tarea principal ejecutándose en el núcleo: %d con prioridad %d \n", xPortGetCoreID(), uxTaskPriorityGet(NULL));

    // Tarea para correr una vez con prioridad alta
    xTaskCreatePinnedToCore(
        startTask2,
        "Task 2",
        1024,
        NULL,
        3,
        &task_2,
        app_cpu
    );

    // Tarea para correr indefinidamente
    xTaskCreatePinnedToCore(
        startTask1,
        "Task 1",
        1024,
        NULL,
        2,
        &task_1,
        app_cpu
    );

    // En vainilla FreeRTOS se debe llamar vTaskStartScheduler() en el main después de configurar las tareas.

    while(1)
    {
        vTaskDelay(15000/portTICK_PERIOD_MS);
        // Suspender tarea de alta prioridad por algunos intervalos
        // for (int i = 0; i < 3; i++)
        // {
        //     vTaskSuspend(task_2);
        //     vTaskDelay(2000/portTICK_PERIOD_MS);
        //     vTaskResume(task_2);
        //     vTaskDelay(2000/ portTICK_PERIOD_MS);
        // }

        // Eliminar tarea de baja prioridad
        if(task_1 != NULL)              // nunca eliminar dos veces
        {
            vTaskDelete(task_1);
            task_1 = NULL;
        }
        vTaskDelay(1);
    }
}
