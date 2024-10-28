/*
 * Resumen:
 * Este código muestra cómo utilizar FreeRTOS en ESP32
 * para reservar memoria.
 * Documentacion: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/09-Memory-management/01-Memory-management
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 27/05/2024
 * Autores: Espindola Agustin, Glas Sebastian
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

// Definir número de núcleo
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Tarea
void testTask(void *parameter)
{
    while(1)
    {
        uint16_t a = 1;
        uint16_t b[100];            // Intentar reservar menos memoria para la tarea para que falle

        // Hacer algo con el array para que no sea optimizado por el compilador
        for(uint16_t i = 0; i < 100; i++)
        {
            b[i] = a + 1;
        }

        printf("%d ", b[0]);

        // Obtener stack restante (palabras)
        printf("Marca de agua alta (palabras): %d\n", uxTaskGetStackHighWaterMark(NULL));
        // Obtener tamaño restante del heap (bytes)
        printf("Memoria heap antes de malloc (bytes): %d\n", xPortGetFreeHeapSize());

        // Funciones para reservar memoria de heap (descomentar una para seleccionar la función utilizada)
        //int *ptr = (int*) malloc(1024 * sizeof(int));     // en ESP32 se llama a heap_caps_malloc, en FreeRTOS vainilla no es thread safe
        //int *ptr = (int *) heap_caps_malloc(1024 * sizeof(int), MALLOC_CAP_DEFAULT);      // esta función se usa en ESP32
        int *ptr = (int*) pvPortMalloc(1024 * sizeof(int));      // función de FreeRTOS vainilla

        // Prevenir overflow de heap corroborando que el retorno de malloc != NULL
        if(ptr == NULL)
        {
            printf("No hay suficiente heap.\n");
        }
        else
        {
            for(uint16_t i = 0; i < 1024; i++)
            {
                ptr[i] = 3;
            }
        }

        // Obtener tamaño restante del heap después de la llamada a malloc (bytes)
        printf("Memoria heap después de malloc (bytes): %d\n", xPortGetFreeHeapSize());

        // Funciones para liberar memoria de heap (intentar comentar esta sección para que se quede sin heap)
        //free(ptr);                // en ESP32 se llama a heap_caps_free, en FreeRTOS vainilla no es thread safe
        //heap_caps_free(ptr);      // función de ESP32 para liberar heap
        vPortFree(ptr);             // función de FreeRTOS vainilla para liberar

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    // Esperar un momento
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("---Demostración de Memoria FreeRTOS---\n");

    // Crear tarea (tamaño de pila insuficiente, se requieren 768 para overhead, para esta tarea mínimo ~1850)
    xTaskCreatePinnedToCore(testTask, "Test Task", 2000, NULL, 1, NULL, app_cpu);

    // En FreeRTOS vainilla se debe llamar vTaskStartScheduler() en el main después de configurar las tareas.

    // Eliminar tarea principal
    vTaskDelete(NULL);
}
