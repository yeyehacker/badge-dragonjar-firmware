#pragma once

#include <stdint.h>
#include <stdbool.h>

/** API pública del módulo Deauthalizer (time-slicing) */
void wifi_deauthalizer_start(void);   
void wifi_deauthalizer_stop(void);    
void wifi_deauthalizer_toggle_pause(void); 
bool wifi_deauthalizer_is_running(void);

// Nota: No necesitamos los setters aquí si no los usamos desde otros módulos.
