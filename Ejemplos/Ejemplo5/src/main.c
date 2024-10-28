/*
 * Resumen:
 * Este código muestra cómo usar un mutex (semaforo) en FreeRTOS
 * Documentación: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/04-Mutexes
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 27/05/2024
 * Autores: Espindola Agustin, Glas Sebastian
 * 
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "FreeRTOSConfig.h"

#include "freertos/task.h"
#include "freertos/semphr.h"

// Definir el número de núcleo a utilizar
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Globales
static int shared_val = 0;
static SemaphoreHandle_t mutex;

//---------------------------------------------------------
// Tareas

// Incrementar la variable compartida (emulando un proceso)
void incTask(void *parameters)
{
    int local_var;

    // Bucle infinito
    while(1)
    {
        if(xSemaphoreTake(mutex, 0) == pdTRUE)
        {
            local_var = shared_val;     // Leer el valor compartido
            local_var++;                // Incrementar el valor
            vTaskDelay((rand() % (500 - 100 + 1) + 100) / portTICK_PERIOD_MS);
            shared_val = local_var;
            // Imprimir el nuevo valor
            printf("Valor Compartido = %d\n", shared_val);

            xSemaphoreGive(mutex);
        }
        else
        {
            // Hacer otra cosa
            vTaskDelay(1);
        }
    }
}

void app_main() 
{
    // Esperar
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("---Demostración de Mutex FreeRTOS---\n");

    // Crear el mutex antes de iniciar las tareas
    mutex = xSemaphoreCreateMutex();

    // Iniciar la tarea 1
    xTaskCreatePinnedToCore(incTask, "Task1", 2000, NULL, 1, NULL, app_cpu);
    // Iniciar la tarea 2
    xTaskCreatePinnedToCore(incTask, "Task2", 2000, NULL, 1, NULL, app_cpu);
}
```