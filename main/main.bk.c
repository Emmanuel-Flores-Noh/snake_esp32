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

// Funcion para renderizar en la pantalla
void IRAM_ATTR renderizar(void *params) {
	int8_t blink = 1;
	while(true) {
		hagl_clear(display);

		if(estadoJuego == MENU) {
			hagl_put_text(display, u"SNAKE ESP32", display -> width / 2 - 30, display -> height / 2 - 18, blanco, font6x9);
			hagl_put_text(display, u"Presiona para iniciar", display -> width / 2 - 60, display -> height / 2, 
			blink == 1 ? blanco : negro, font6x9);
			blink *= -1;
		}
		else if(estadoJuego == JUGANDO || estadoJuego == PAUSE) {
			hagl_fill_rectangle(display, 0, 0, display -> width, display -> height, blanco);
			for(uint8_t i = 0; i < contadorSecciones; i++) {
				hagl_fill_rectangle(display, serpiente[i].x * 16, serpiente[i].y * 16, (serpiente[i].x + 1) * 16, (serpiente[i].y + 1) * 16, cyan);
			}
		}
		else if(estadoJuego == GAMEOVER) {
			hagl_put_text(display, u"GAMEOVER", display -> width / 2 - 30, display -> height / 2 - 18, blanco, font6x9);
		}
		hagl_flush(display);
		vTaskDelay(pdTICKS_TO_MS(10));
	}
}

struct {
	uint8_t x,y;
} joystick;

uint8_t boton;

// Funcion para manejar los input
void input(void *param) {
	while(true) {
		joystick.x = adc1_get_raw(JOYSTICK_PIN_X);
		joystick.y = adc1_get_raw(JOYSTICK_PIN_Y);
		boton = gpio_get_level(JOYSTICK_PIN_BUTTON);
		printf("%i %i %i\n", joystick.x, joystick.y, boton);

		if(estadoJuego == MENU && boton == 0) {
			estadoJuego = JUGANDO;
		}
		if(estadoJuego == JUGANDO) {
			if(boton == 0) {
				estadoJuego = PAUSE;
				vTaskDelay(pdMS_TO_TICKS(200));
				continue;
			}

			switch(direccion) {
				case ARRIBA:
				case ABAJO:
					if(joystick.x <= 0) {
						direccion = IZQUIERDA;
					}
					else if(joystick.x >= 255) {
						direccion = DERECHA;
					}
					break;
				case IZQUIERDA:
				case DERECHA:
					if(joystick.y <= 0) {
						direccion = ARRIBA;
					}
					else if(joystick.y >= 255) {
						direccion = ABAJO;
					}
					break;
			}


		}

		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void app_main(void)
{
	configurarPines();
	display = hagl_init();

	xTaskCreatePinnedToCore(renderizar, "Renderizado", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
	xTaskCreatePinnedToCore(input, "Input", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL, 0);

	estadoJuego = MENU;
	
	while(true) {

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
