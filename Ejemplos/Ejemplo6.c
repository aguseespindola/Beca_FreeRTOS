/**
 * 
 * Resumen:
 * Demuestra un semáforo contador creando varias tareas con los mismos
 * parámetros.
 * Documentacion: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/03-Counting-semaphores
 * 
 * Configuración GPIO:
 * GPIO1 y 3 usados para UART0
 *
 * Fecha: 08/08/2024
 * Autores: Espindola Agustin, Glas Sebastian
 * 
 */

// Probablemente necesitarás esto en FreeRTOS vainilla
//#include <semphr.h>

#include "freertos/FreeRTOS.h"
#include <string.h>

// Definir el número de núcleo a utilizar
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Configuraciones
static const int num_tasks = 5;     // Número de tareas a crear

// Ejemplo de estructura para pasar una cadena como parámetro
typedef struct Message {
  char body[20];
  uint8_t len;
} Message;

// Variables globales
static SemaphoreHandle_t sem_params; // Cuenta regresiva cuando los parámetros son leídos

//*****************************************************************************
// Tareas

void myTask(void *parameters) {

    // Copiar la estructura de mensaje desde el parámetro a una variable local
    Message msg = *(Message *)parameters;

    // Incrementar el semáforo para indicar que el parámetro ha sido leído
    xSemaphoreGive(sem_params);

    // Imprimir el contenido del mensaje
    printf("Recibido: %s | len: %d \n", msg.body, msg.len);

    // Esperar un momento y eliminarse a sí mismo
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

//*****************************************************************************
// Main (se ejecuta como su propia tarea con prioridad 1 en el núcleo 1)
void app_main() {
    char task_name[12];
    Message msg;
    char text[20] = "All your base";
    
    // Configurar Serial
    //

    // Esperar un momento para comenzar (para no perder la salida Serial)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n---Demostración de Semáforo Contador en FreeRTOS---\n");

    // Crear semáforos (inicializados en 0)
    sem_params = xSemaphoreCreateCounting(num_tasks, 0);

    // Crear mensaje para usar como argumento común para todas las tareas
    strcpy(msg.body, text);
    msg.len = strlen(text);

    // Iniciar tareas
    for (int i = 0; i < num_tasks; i++) {
        // Generar una cadena de nombre única para la tarea
        sprintf(task_name, "Tarea %i", i);

        // Iniciar la tarea y pasar el argumento (estructura Message común)
        xTaskCreatePinnedToCore(myTask,
                                task_name,
                                1700,
                                (void *)&msg,
                                1,
                                NULL,
                                app_cpu);
    }

    // Esperar a que todas las tareas lean la memoria compartida
    for (int i = 0; i < num_tasks; i++) {
        xSemaphoreTake(sem_params, portMAX_DELAY);
    }

    // Notificar que todas las tareas han sido creadas
    printf("Todas las tareas han sido creadas\n");

    while(1){
        // No hacer nada, pero permitir ceder a tareas de menor prioridad
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
