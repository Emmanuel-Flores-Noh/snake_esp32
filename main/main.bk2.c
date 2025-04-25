#include <stdio.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <hagl.h>
#include <hagl_hal.h>
#include "sdkconfig.h"
#include <wchar.h>
#include <hal/adc_types.h>
#include <time.h>

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

// Algunas configuraciones
#define inputBufferSize 2

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
	int8_t x, y;
} coords;

#define serpienteAncho 16
#define serpienteAlto 16
#define tableroAncho 10
#define tableroAlto 8

coords serpiente[80] = {
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

enum direcciones {
	ARRIBA,
	ABAJO,
	IZQUIERDA,
	DERECHA
};
enum direcciones direccion = DERECHA;

coords fruta = {
	.x = -1,
	.y = -1
};

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

			hagl_fill_rectangle(display, fruta.x * 16, fruta.y * 16, (fruta.x + 1) * 16, (fruta.y + 1) * 16, negro);

			if(estadoJuego == PAUSE) {
				hagl_put_text(display, u"PAUSE", display -> width / 2 - 15, display -> height / 2 - 9, 
				blink == 1 ? blanco : negro, font6x9);
				blink *= -1;
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

enum direcciones inputBuffer[inputBufferSize] = {-1, -1};

void input(void *param) {
	while(true) {
		joystick.x = adc1_get_raw(JOYSTICK_PIN_X);
		joystick.y = adc1_get_raw(JOYSTICK_PIN_Y);
		boton = gpio_get_level(JOYSTICK_PIN_BUTTON);
		printf("%i %i %i\n", joystick.x, joystick.y, boton);

		if(estadoJuego == MENU && boton == 0) {
			while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
			estadoJuego = JUGANDO;
		}
		else if(estadoJuego == JUGANDO) {

			if(boton == 0) {
				estadoJuego = PAUSE;
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
				continue;
			}

			enum direcciones* buffer = NULL;

			// Comprueba si se va a guardar el input
			for(int i = 0; i < inputBufferSize; i++) {
				if(inputBuffer[i] != -1) {
					continue;
				}
				buffer = &inputBuffer[i];
				break;
			}

			if(buffer == NULL) {
				// No se pueden guardar mas inputBuffers
				vTaskDelay(pdMS_TO_TICKS(300));
				continue;
			}

			switch(direccion) {
				case ARRIBA:
				case ABAJO:
					if(joystick.x <= 0) {
						*buffer = IZQUIERDA;
					}
					else if(joystick.x >= 255) {
						*buffer = DERECHA;
					}
					break;
				case IZQUIERDA:
				case DERECHA:
					if(joystick.y <= 0) {
						*buffer = ARRIBA;
					}
					else if(joystick.y >= 255) {
						*buffer = ABAJO;
					}
					break;
			}


		}
		else if(estadoJuego == PAUSE) {
			if(boton == 0) {
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
				estadoJuego = JUGANDO;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void movimientoSerpiente() {
	for(uint8_t i = contadorSecciones - 1; 0 < i ; i--) {
		serpiente[i].x = serpiente[i - 1].x;
		serpiente[i].y = serpiente[i - 1].y;
	}

	if(inputBuffer[0] != -1) {
		direccion = inputBuffer[0];
		for(int i = 1; i < inputBufferSize; i++) {
			inputBuffer[i - 1] = inputBuffer[i];
		}
		inputBuffer[inputBufferSize - 1] = -1;
	}

	switch(direccion) {
		case ARRIBA:
			serpiente[0].y -= 1;
			break;
		case ABAJO:
			serpiente[0].y += 1;
			break;
		case IZQUIERDA:
			serpiente[0].x -= 1;
			break;
		case DERECHA:
			serpiente[0].x += 1;
			break;
	}
}

void generarFruta() {
	fruta.x = rand() % tableroAncho;
	fruta.y = rand() % tableroAlto;
}

void comprobarComer() {
	if(fruta.x == serpiente[0].x && fruta.y == serpiente[0].y) {
		generarFruta();
		printf("Fruta comida!\n");
	}
}


void app_main(void)
{
	configurarPines();
	display = hagl_init();

	xTaskCreatePinnedToCore(renderizar, "Renderizado", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
	xTaskCreatePinnedToCore(input, "Input", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL, 0);

	estadoJuego = MENU;
	// Time retorna unix epoch
	// Srand pone la semilla para la aleatoriedad
	srand(time(NULL));

	generarFruta();
	
	while(true) {
		if(estadoJuego == JUGANDO) {
			movimientoSerpiente();
			comprobarComer();
		}



		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
