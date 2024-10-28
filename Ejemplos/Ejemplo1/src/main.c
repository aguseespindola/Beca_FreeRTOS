/*
 * Resumen:
 * Este código muestra cómo utilizar FreeRTOS en ESP32 generando el parpadeo de un LED.
 * Documentacion: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines
 *
 * GPIO:
 * GPIO2: Salida conectada al LED
 *
 * Fecha: 23/05/2024
 * Autores: Espindola Agustin, Glas Sebastian
 */
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Definir el núcleo a utilizar
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// GPIO
static const int GPIO_LED = GPIO_NUM_2;

// Definición de tareas (parpadeo del LED)
void toggleLED(void *parameter)
{
    while(1)
    {
         gpio_set_level(GPIO_LED, 0);
        vTaskDelay(500/portTICK_PERIOD_MS);
         gpio_set_level(GPIO_LED, 1);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}

void app_main() 
{
    // Estructura de configuración de GPIO
    //(https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html#_CPPv413gpio_config_t)
    gpio_config_t io_config = {
        .pin_bit_mask = (1<<GPIO_LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    // Función para configurar GPIO
    gpio_config(&io_config);

    // Creación de tareas
    xTaskCreatePinnedToCore(
        toggleLED,      // Función a llamar
        "Toggle LED",   // Nombre de la tarea
        1024,           // Tamaño de la pila (bytes en ESP32, palabras en FreeRTOS)
        NULL,           // Parámetro para pasar a la función
        1,              // Prioridad de la tarea (0 a configMAX_PRIORITIES - 1)
        NULL,           // Identificador de la tarea
        app_cpu);       // CPU para ejecutar la tarea (ESP32)

    // En FreeRTOS vainilla se debe llamar vTaskStartScheduler() en el main después de configurar las tareas.
}

