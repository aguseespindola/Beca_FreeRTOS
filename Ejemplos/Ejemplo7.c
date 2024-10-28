/**
 * 
 * Resumen:
 * Demuestra el uso básico de temporizadores de software.
 * Documentacion: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/05-Software-timers/01-Software-timers
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 08/08/2024
 * Autores: Espindola Agustin, Glas Sebastian
 * 
 */

// Probablemente necesitarás esto en FreeRTOS vainilla
//#include <timers.h>

#include "freertos/FreeRTOS.h"

// Usar solo el núcleo 1 para propósitos de demostración
// #if CONFIG_FREERTOS_UNICORE
//   static const BaseType_t app_cpu = 0;
// #else
//   static const BaseType_t app_cpu = 1;
// #endif

// Variables globales
static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;

//*****************************************************************************
// Callbacks

// Llamado cuando uno de los temporizadores expira
void myTimerCallback(TimerHandle_t xTimer) {

  // Imprimir mensaje si el temporizador 0 expira
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0) {
    printf("Temporizador de una sola vez expiró\n");
  }

  // Imprimir mensaje si el temporizador 1 expira
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1) {
    printf("Temporizador de recarga automática expiró\n");
  }
}

//*****************************************************************************
// Main (se ejecuta como su propia tarea con prioridad 1 en el núcleo 1)

void app_main() {
    // Configurar Serial


    // Esperar un momento para comenzar (para no perder la salida Serial)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n");
    printf("---Demostración de Temporizador en FreeRTOS---\n");

    // Crear un temporizador de una sola vez
    one_shot_timer = xTimerCreate(
                        "Temporizador de una sola vez",  // Nombre del temporizador
                        2000 / portTICK_PERIOD_MS,       // Periodo del temporizador (en ticks)
                        pdFALSE,                         // Recarga automática
                        (void *)0,                       // ID del temporizador
                        myTimerCallback);                // Función de callback

    // Crear un temporizador de recarga automática
    auto_reload_timer = xTimerCreate(
                        "Temporizador de recarga automática", // Nombre del temporizador
                        1000 / portTICK_PERIOD_MS,            // Periodo del temporizador (en ticks)
                        pdTRUE,                               // Recarga automática
                        (void *)1,                            // ID del temporizador
                        myTimerCallback);                     // Función de callback

    // Verificar que los temporizadores se hayan creado
    if (one_shot_timer == NULL || auto_reload_timer == NULL) {
        printf("No se pudo crear uno de los temporizadores\n");
    } else {
        
        // Esperar y luego imprimir un mensaje de que estamos iniciando los temporizadores
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Iniciando temporizadores...\n");

        // Iniciar temporizadores (tiempo máximo de bloqueo si la cola de comandos está llena)
        xTimerStart(one_shot_timer, portMAX_DELAY);
        xTimerStart(auto_reload_timer, portMAX_DELAY);
    }

    // Eliminar la tarea propia para demostrar que los temporizadores funcionarán sin tareas de usuario
    vTaskDelete(NULL);

    while(1){
        // La ejecución nunca debería llegar aquí
    }
}