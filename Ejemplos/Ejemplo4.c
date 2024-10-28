/*
 * Resumen:
 * Este código muestra cómo usar FreeRTOS con una demostración de colas.
 * Documentación: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues
 * 
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 27/05/2024
 * Autores: Espindola Agustin, Glas Sebastian
 * Documentación: https://www.freertos.org/a00018.html
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Definir el número de núcleo a utilizar
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Configuraciones
#define MSG_QUEUE_LEN 5

// Global
static QueueHandle_t msg_queue;

//------------------------------------------------------
// Tareas

// Tarea1
void printMsg(void *parameters)
{
    int item;
    
    // Bucle infinito
    mientras(1)
    {
        // Ver si hay un mensaje en la cola (no bloquear)
        if(xQueueReceive(msg_queue, (void*)&item, 0) == pdTRUE)
        {
            printf("%d\n", item);
        }
        // Esperar antes de intentarlo de nuevo
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

// main
void app_main() 
{
    // Esperar un momento
    vTaskDelay(1000/portTICK_PERIOD_MS);

    printf("-----Demostración de Cola FreeRTOS-----\n");

    // Crear cola
    msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(int));
    if(msg_queue == NULL)
    {
        printf("ERROR: handle de cola NULL\n");
    }

    // Iniciar tarea de impresión
    xTaskCreatePinnedToCore(printMsg, "Print Msg", 1800, NULL, 1, NULL, app_cpu);

    while(1)
    {
        static int num = 0;

        // Intentar agregar elemento a la cola durante 10 ticks, falla si la cola está llena.
        if(xQueueSend(msg_queue, (void*)&num, 10) != pdTRUE)
        {
            printf("ERROR: Cola llena\n");
        }
        num++;
        
        // Jugar con el tiempo para llenar completamente la cola
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
