/**
 * 
 * Resumen:
 * Demostrar un deadlock con 2 tareas.
 * Documentacion: https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-10-deadlock-and-starvation/872c6a057901432e84594d79fcb2cc5d
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

// Define uno para seleccionar la ejecución del código
#define DEADLOCK_EN
//#define DEADLOCK_TIMEOUT
//#define DEADLOCK_HIERARCHY

// Usar solo el núcleo 1 para propósitos de demostración
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Configuración
TickType_t mutex_timeout = 1000 / portTICK_PERIOD_MS;

// Variables globales
static SemaphoreHandle_t mutex_1;
static SemaphoreHandle_t mutex_2;

//*****************************************************************************
// Tareas

// Tarea A (alta prioridad)
void doTaskA(void *parameters) {

  // Bucle infinito
  while (1) {

    #ifdef DEADLOCK_EN  // Deadlock habilitado

        // Tomar el mutex 1 (introduce espera para forzar el deadlock)
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        printf("Tarea A tomó mutex 1\n");
        vTaskDelay(100 / portTICK_PERIOD_MS);

        // Tomar el mutex 2
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        printf("Tarea A tomó mutex 2\n");

        // Sección crítica protegida por 2 mutexes
        printf("Tarea A haciendo algún trabajo\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Devolver mutexes
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);
        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea A se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

    #elif defined(DEADLOCK_TIMEOUT)   // Deadlock deshabilitado con timeout
        // Tomar el mutex 1
        if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {

        // Indicar que tomamos el mutex 1 y esperar (para forzar el deadlock)
        printf("Tarea A tomó mutex 1\n");
        vTaskDelay(1 / portTICK_PERIOD_MS);
    
        // Tomar el mutex 2
        if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {

            // Indicar que tomamos el mutex 2
            printf("Tarea A tomó mutex 2\n");
    
            // Sección crítica protegida por 2 mutexes
            printf("Tarea A haciendo algún trabajo\n");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            printf("Tarea A se agotó esperando por el mutex 2\n");
        }
        } else {
        printf("Tarea A se agotó esperando por el mutex 1\n");
        }

        // Devolver mutexes
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);

        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea A se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    
    #elif defined(DEADLOCK_HIERARCHY)   // Deadlock deshabilitado usando jerarquía de semáforos
        // Tomar el mutex 1 (introduce espera para forzar el deadlock)
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        printf("Tarea A tomó mutex 1\n");
        vTaskDelay(1 / portTICK_PERIOD_MS);

        // Tomar el mutex 2
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        printf("Tarea A tomó mutex 2\n");

        // Sección crítica protegida por 2 mutexes
        printf("Tarea A haciendo algún trabajo\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Devolver mutexes (en orden inverso al que los tomamos)
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);

        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea A se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    #endif
  }
}

// Tarea B (baja prioridad)
void doTaskB(void *parameters) {

  // Bucle infinito
  while (1) {
    #if defined(DEADLOCK_EN)  // Deadlock habilitado

        // Tomar el mutex 2 (introduce espera para forzar el deadlock)
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        printf("Tarea B tomó mutex 2\n");
        vTaskDelay(100 / portTICK_PERIOD_MS);

        // Tomar el mutex 1
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        printf("Tarea B tomó mutex 1\n");

        // Sección crítica protegida por 2 mutexes
        printf("Tarea B haciendo algún trabajo\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Devolver mutexes
        xSemaphoreGive(mutex_1);
        xSemaphoreGive(mutex_2);
        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea A se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

    #elif defined(DEADLOCK_TIMEOUT)   // Deadlock deshabilitado usando timeouts
    
        // Tomar el mutex 2
        if (xSemaphoreTake(mutex_2, mutex_timeout) == pdTRUE) {

        // Indicar que tomamos el mutex 2 y esperar (para forzar el deadlock)
        printf("Tarea B tomó mutex 2\n");
        vTaskDelay(1 / portTICK_PERIOD_MS);
    
        // Tomar el mutex 1
        if (xSemaphoreTake(mutex_1, mutex_timeout) == pdTRUE) {

            // Indicar que tomamos el mutex 1
            printf("Tarea B tomó mutex 1\n");
    
            // Sección crítica protegida por 2 mutexes
            printf("Tarea B haciendo algún trabajo\n");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            printf("Tarea B se agotó esperando por el mutex 1\n");
        }
        } else {
        printf("Tarea B se agotó esperando por el mutex 2\n");
        }

        // Devolver mutexes
        xSemaphoreGive(mutex_1);
        xSemaphoreGive(mutex_2);

        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea B se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
    #elif defined(DEADLOCK_HIERARCHY)   // Deadlock deshabilitado usando jerarquía de semáforos
        // Tomar el mutex 1 (introduce espera para forzar el deadlock)
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        printf("Tarea B tomó mutex 1\n");
        vTaskDelay(1 / portTICK_PERIOD_MS);

        // Tomar el mutex 2
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        printf("Tarea B tomó mutex 2\n");

        // Sección crítica protegida por 2 mutexes
        printf("Tarea B haciendo algún trabajo\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Devolver mutexes (en orden inverso al que los tomamos)
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);

        // Esperar para dejar que la otra tarea se ejecute
        printf("Tarea A se va a dormir\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    #endif
  }
}

//*****************************************************************************
// Main (se ejecuta como su propia tarea con prioridad 1 en el núcleo 1)

void app_main() {
    // Configurar Serial

    // Esperar un momento para comenzar (para no perder la salida Serial)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n---Demostración de Deadlock en FreeRTOS---\n");

    // Crear mutexes antes de iniciar las tareas
    mutex_1 = xSemaphoreCreateMutex();
    mutex_2 = xSemaphoreCreateMutex();

    // Iniciar Tarea A (alta prioridad)
    xTaskCreatePinnedToCore(doTaskA,
                            "Tarea A",
                            1024,
                            NULL,
                            2,
                            NULL,
                            app_cpu);

    // Iniciar Tarea B (baja prioridad)
    xTaskCreatePinnedToCore(doTaskB,
                            "Tarea B",
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);


    // Eliminar tarea "setup and loop"
    vTaskDelete(NULL);
    while(1){
        // La ejecución nunca debería llegar aquí
    }
}