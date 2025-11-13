#include "deauthalizer_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "deauth_module.h" // Para usar deauth_module_begin
#include "wifi_module.h" // Para usar funciones del analizador (asumidas)

/* * Renombramiento de funciones existentes basado en menus.h:
 * Analizer: la función de inicio es wifi_module_analyzer_run()
 * Deauth: la función de inicio es deauth_module_begin()
 * Necesitamos una API de STOP para ambas, que no están visibles en menus.h.
 * Por ahora, usaremos stubs y FreeRTOS para el time-slicing.
 */

// Stubs para stop (debes implementar estas en los módulos reales si no existen)
void wifi_module_analyzer_stop(void) { ESP_LOGW("DEAUTHALIZER", "wifi_module_analyzer_stop needs implementation!"); }
void deauth_module_stop(void) { ESP_LOGW("DEAUTHALIZER", "deauth_module_stop needs implementation!"); }


#define TAG "DEAUTHALIZER"
#define DEAUTHALIZER_TASK_STACK 4096

// Valores por defecto
static uint32_t s_deauth_ms = 300;
static uint32_t s_analyzer_ms = 700;

// Control por EventGroup
static EventGroupHandle_t s_evt = NULL;
// bits
#define BIT_STOP    (1 << 0)
#define BIT_PAUSE   (1 << 1)
#define BIT_RUNNING (1 << 2)

static TaskHandle_t s_task = NULL;

static inline bool stop_requested(void) {
    if (!s_evt) return false;
    return (xEventGroupGetBits(s_evt) & BIT_STOP) != 0;
}

static inline bool pause_requested(void) {
    if (!s_evt) return false;
    return (xEventGroupGetBits(s_evt) & BIT_PAUSE) != 0;
}

/* La tarea de time-slicing */
static void deauthalizer_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Deauthalizer task started (300ms Deauth / 700ms Analyze)");
    xEventGroupSetBits(s_evt, BIT_RUNNING);

    while (!stop_requested()) {
        if (pause_requested()) {
            ESP_LOGI(TAG, "Paused. Waiting resume/stop...");
            xEventGroupWaitBits(s_evt, BIT_STOP | BIT_PAUSE, pdFALSE, pdFALSE, portMAX_DELAY);
            continue;
        }

        /* 1) Ráfaga de Deauth (usa la API de inicio existente) */
        ESP_LOGI(TAG, "Deauth burst: %u ms", (unsigned)s_deauth_ms);
        deauth_module_begin(); // Usamos la función del menú (que debe ser de inicio)
        vTaskDelay(pdMS_TO_TICKS(s_deauth_ms));
        deauth_module_stop(); // Usamos el stub

        if (stop_requested()) break;
        if (pause_requested()) continue;

        /* 2) Ventana de Analyzer (usa la API de inicio existente) */
        ESP_LOGI(TAG, "Analyzer window: %u ms", (unsigned)s_analyzer_ms);
        wifi_module_analyzer_run(); // Usamos la función de inicio del menú
        vTaskDelay(pdMS_TO_TICKS(s_analyzer_ms));
        wifi_module_analyzer_stop(); // Usamos el stub
    }

    ESP_LOGI(TAG, "Deauthalizer task stopping");
    xEventGroupClearBits(s_evt, BIT_RUNNING);
    s_task = NULL;
    vTaskDelete(NULL);
}

/* API pública (llamadas desde menus.h) */
void wifi_deauthalizer_start(void)
{
    if (s_task != NULL) {
        ESP_LOGI(TAG, "Deauthalizer already running");
        return;
    }
    if (s_evt == NULL) {
        s_evt = xEventGroupCreate();
        if (s_evt == NULL) return;
    }
    xEventGroupClearBits(s_evt, BIT_STOP | BIT_PAUSE);
    xTaskCreate(deauthalizer_task, "deauthalizer", DEAUTHALIZER_TASK_STACK, NULL, 5, &s_task);
}

void wifi_deauthalizer_stop(void)
{
    if (!s_evt) return;
    xEventGroupSetBits(s_evt, BIT_STOP);
}

void wifi_deauthalizer_toggle_pause(void)
{
    if (!s_evt) return;
    EventBits_t bits = xEventGroupGetBits(s_evt);
    if (bits & BIT_PAUSE) {
        xEventGroupClearBits(s_evt, BIT_PAUSE); // resume
    } else {
        xEventGroupSetBits(s_evt, BIT_PAUSE); // pause
    }
}

bool wifi_deauthalizer_is_running(void)
{
    if (!s_evt) return false;
    return (xEventGroupGetBits(s_evt) & BIT_RUNNING) != 0;
}
