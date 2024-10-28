/**
 * 
 * Resumen:
 * Demostrar inversión de prioridades.
 * Documentacion: https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-11-priority-inversion/abf4b8f7cd4a4c70bece35678d178321
 *
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 08/08/2024
 * Autores: Espindola Agustin, Glas Sebastian
 * 
 */
 
// Probablemente necesitarás esto en FreeRTOS estándar
//#include <semphr.h>

#include "freertos/FreeRTOS.h"
#include "esp_task_wdt.h"

// Usar solo el núcleo 1 para propósitos de demostración
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Comentar para habilitar la herencia de prioridades y resolver el problema
#define PRIO_INVERSION_EN

// Configuración
TickType_t cs_wait = 250;   // Tiempo pasado en la sección crítica (ms)
TickType_t med_wait = 5000; // Tiempo que la tarea de prioridad media pasa trabajando (ms)

// Variables globales
static SemaphoreHandle_t lock;

//*****************************************************************************
// Tareas

// Tarea L (baja prioridad)
void doTaskL(void *parameters) {

  TickType_t timestamp;

  // Hacer para siempre
  while (1) {

    // Tomar el bloqueo
    printf("Tarea L intentando tomar el bloqueo...\n");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    // Indicar cuánto tiempo pasamos esperando el bloqueo
    printf("Tarea L obtuvo el bloqueo. Pasó %lu ms esperando el bloqueo. Haciendo algo de trabajo...\n", (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);

    // Acaparar el procesador durante un tiempo sin hacer nada
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    // Liberar el bloqueo
    printf("Tarea L liberando el bloqueo.\n");
    xSemaphoreGive(lock);

    // Ir a dormir
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// Tarea M (prioridad media)
void doTaskM(void *parameters) {

  TickType_t timestamp;

  // Hacer para siempre
  while (1) {

    // Acaparar el procesador durante un tiempo sin hacer nada
    printf("Tarea M haciendo algo de trabajo...\n");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < med_wait);

    // Ir a dormir
    printf("¡Tarea M terminada!\n");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// Tarea H (alta prioridad)
void doTaskH(void *parameters) {

  TickType_t timestamp;

  // Hacer para siempre
  while (1) {

    // Tomar el bloqueo
    printf("Tarea H intentando tomar el bloqueo...\n");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    // Indicar cuánto tiempo pasamos esperando el bloqueo
    printf("Tarea H obtuvo el bloqueo. Pasó %lu ms esperando el bloqueo. Haciendo algo de trabajo...\n", (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);

    // Acaparar el procesador durante un tiempo sin hacer nada
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ( (xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    // Liberar el bloqueo
    printf("Tarea H liberando el bloqueo.\n");
    xSemaphoreGive(lock);
    
    // Ir a dormir
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

//*****************************************************************************
// Principal (se ejecuta como su propia tarea con prioridad 1 en el núcleo 1)

void app_main() {
    // Configurar Serial

    // Esperar un momento para comenzar (para no perder la salida Serial)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n---Demostración de Inversión de Prioridades en FreeRTOS---\n");

    // Crear semáforos y mutexes antes de iniciar las tareas
    #ifdef PRIO_INVERSION_EN
      lock = xSemaphoreCreateBinary();
      xSemaphoreGive(lock); // Asegurarse de que el semáforo binario comience en 1
    #else
      lock = xSemaphoreCreateMutex();
    #endif
    

    // El orden de inicio de las tareas importa para forzar la inversión de prioridades

    // Iniciar Tarea L (baja prioridad)
    xTaskCreatePinnedToCore(doTaskL,
                            "Tarea L",
                            2024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
    
    // Introducir una demora para forzar la inversión de prioridades
    vTaskDelay(1);

    // Iniciar Tarea H (alta prioridad)
    xTaskCreatePinnedToCore(doTaskH,
                            "Tarea H",
                            2024,
                            NULL,
                            3,
                            NULL,
                            app_cpu);

    // Iniciar Tarea M (prioridad media)
    xTaskCreatePinnedToCore(doTaskM,
                            "Tarea M",
                            2024,
                            NULL,
                            2,
                            NULL,
                            app_cpu);

    // Eliminar tarea "setup and loop"
    vTaskDelete(NULL);

    while(1){
        // La ejecución nunca debería llegar aquí
    }
}