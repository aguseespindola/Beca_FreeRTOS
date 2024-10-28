/**
 * 
 * Resumen:
 * Leer valores de ADC en ISR a 1 Hz y diferir la impresión de ellos en una tarea.
 * Documentacion: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_oneshot.html
				https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html
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
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"

// Usar solo el núcleo 1 para propósitos de demostración
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Configuración
#define TIMER_DIVIDER         (80)  // Divisor de reloj del temporizador de hardware
#define TIMER_SCALE           (80000000 / TIMER_DIVIDER)  // convertir valor del contador a segundos

// Pines
#define ADC_EXAMPLE_CHAN      ADC_CHANNEL_0

// Variables globales
adc_oneshot_unit_handle_t adc1_handle;
int val;
SemaphoreHandle_t bin_sem = NULL;

//*****************************************************************************
// Rutinas de Servicio de Interrupciones (ISRs)

/**
 * @brief ISR del Temporizador
 */
static void IRAM_ATTR onTimer() {
  BaseType_t task_woken = pdFALSE;

  // Realizar acción (leer del ADC)
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_EXAMPLE_CHAN, &val));

  // Dar semáforo para indicar a la tarea que un nuevo valor está listo
  xSemaphoreGiveFromISR(bin_sem, &task_woken);

  // Salir de la ISR (FreeRTOS estándar)
  //portYIELD_FROM_ISR(task_woken);

  // Salir de la ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

//*****************************************************************************
// Tareas

// 
/**
 * @brief Esperar por semáforo e imprimir el valor de ADC cuando se reciba
 */
void printValues(void *parameters) {

  // Bucle infinito, esperar por semáforo e imprimir valor
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    printf("%d\n", val); 
  }
}

//*****************************************************************************
/**
 * @brief Inicializar el temporizador seleccionado del grupo de temporizadores
 *
 */
static void example_tg_timer_init()
{
  esp_timer_handle_t timer_handle;
  esp_timer_create_args_t timer_config = {
    .callback = &onTimer,
    .name = "periodic"
  };

  ESP_ERROR_CHECK(esp_timer_create(&timer_config, &timer_handle));
  /* Iniciar los temporizadores */
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 1000000));
}

//*****************************************************************************
/**
 * @brief Inicializar el canal ADC seleccionado
 *
 */
static void example_adc_init(void)
{
    //-------------Inicialización de ADC1---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------Configuración de ADC1---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_EXAMPLE_CHAN, &config));

}

//*****************************************************************************
// Main (se ejecuta como su propia tarea con prioridad 1 en el núcleo 1)

void app_main() {
    // Configurar Serial

    // Esperar un momento para comenzar (para no perder la salida Serial)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("\n---Demostración de Buffer ISR en FreeRTOS---\n");

    // Crear semáforo antes de que se use (en tarea o ISR)
    bin_sem = xSemaphoreCreateBinary();

    // Forzar reinicio si no se puede crear el semáforo
    if (bin_sem == NULL) {
        printf("No se pudo crear el semáforo\n");
        esp_restart();
    }

    // Iniciar tarea para imprimir resultados (¡mayor prioridad!)
    xTaskCreatePinnedToCore(printValues,
                            "Imprimir valores",
                            2048,
                            NULL,
                            2,
                            NULL,
                            app_cpu);

    // Inicializar ADC
    example_adc_init();
    // Crear e iniciar temporizador (num, divisor, countUp)
    example_tg_timer_init();
    
    
    while(1){
        // No hacer nada, para siempre
        vTaskDelay(1000);
    }
}