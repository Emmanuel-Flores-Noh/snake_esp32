#include <stdio.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <hagl_hal.h>
#include <hagl.h>
#include "sdkconfig.h"
#include <wchar.h>
#include <hal/adc_types.h>

// Fuente
#include <font6x9.h>

// Colores
#define blanco 0xffff
#define negro 0x0000
#define cyan 0x9c3d

// Pines del joystick Pins 34 y 35
#define JOYSTICK_PIN_X ADC1_CHANNEL_6
#define JOYSTICK_PIN_Y ADC1_CHANNEL_7
#define JOYSTICK_PIN_BUTTON GPIO_NUM_13

// Display
static hagl_backend_t *display;

// Estados del juego
enum estadosJuego {
	MENU,
	JUGANDO,
	PAUSE,
	GAMEOVER
};

enum estadosJuego estadoJuego;

// Secciones de la serpiente
// Sprites de 16 x 16
// tablero de 10 x 8
typedef struct {
	uint8_t x, y;
} seccion;

#define serpienteAncho 16
#define serpienteAlto 16
#define tableroAncho 10
#define tableroAlto 8

seccion serpiente[80] = {
	{
		.x = 3,
		.y = 4
	},
	{
		.x = 2,
		.y = 4
	},
	{
		.x = 1,
		.y = 4
	}
};

uint8_t contadorSecciones = 3;

enum {
	ARRIBA,
	ABAJO,
	IZQUIERDA,
	DERECHA
} direccion = IZQUIERDA;

void configurarPines() {
	// Configurar pines analogicos para el joystick
	adc1_config_channel_atten(JOYSTICK_PIN_X, ADC_ATTEN_DB_12);
	adc1_config_channel_atten(JOYSTICK_PIN_Y, ADC_ATTEN_DB_12);

	// Configurar el pin digital para el boton del joystick
	gpio_config_t joybutton = {
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = 1ULL << JOYSTICK_PIN_BUTTON,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	
	gpio_config(&joybutton);
}

void app_main(void)
{
	configurarPines();
	display = hagl_init();

	estadoJuego = MENU;
	
	while(true) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
