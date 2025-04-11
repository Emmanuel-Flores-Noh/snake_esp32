#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <wchar.h>

#include <hagl_hal.h>
#include <hagl_hal_color.h>
#include <hagl.h>

#include <font6x9.h>

// Pines del joystick Pins 34 y 35
#define JOYSTICK_PIN_X ADC1_CHANNEL_6
#define JOYSTICK_PIN_Y ADC1_CHANNEL_7
#define JOYSTICK_PIN_BUTTON GPIO_NUM_13

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

static hagl_backend_t *display;

struct {int16_t x, y, boton;} joystick;
uint8_t seleccion = 0;
struct {
	uint8_t r, g, b;
} color = {.r = 0, .g = 0, .b = 0};
char selecciones[] = {'r', 'g', 'b'};

void modificarColor(char canal, int modificacion) {
	switch(canal) {
		case 'r':
			if(color.r >= 255 && modificacion > 0) break;
			if(color.r == 0 && modificacion < 0) break;
			color.r += modificacion;
			break;
		case 'g':
			if(color.g >= 255 && modificacion > 0) break;
			if(color.g == 0 && modificacion < 0) break;
			color.g += modificacion;
			break;
		case 'b':
			if(color.b >= 255 && modificacion > 0) break;
			if(color.b == 0 && modificacion < 0) break;
			color.b += modificacion;
			break;
		default:
			break;
	}
}

void input() {
	while(true) {
		joystick.x = adc1_get_raw(JOYSTICK_PIN_X);
		joystick.y = adc1_get_raw(JOYSTICK_PIN_Y);
		joystick.boton = gpio_get_level(JOYSTICK_PIN_BUTTON);
		// printf("%i %i %i\n", joystick.x, joystick.y, joystick.boton);
		// printf("%i\n", seleccion);
	
		while((joystick.y = adc1_get_raw(JOYSTICK_PIN_Y)) >= 4095) {
			modificarColor(selecciones[seleccion], -1);
			vTaskDelay(pdMS_TO_TICKS(50));
		}
	
		while((joystick.y = adc1_get_raw(JOYSTICK_PIN_Y)) <= 0) {
			modificarColor(selecciones[seleccion], 1);
			vTaskDelay(pdMS_TO_TICKS(50));
		}

		while((joystick.x = adc1_get_raw(JOYSTICK_PIN_X)) >= 4095) {
			seleccion = (seleccion + 1) % 3;
			vTaskDelay(pdMS_TO_TICKS(300));
		}
		
		while((joystick.x = adc1_get_raw(JOYSTICK_PIN_X)) <= 0) {
			if(seleccion - 1 < 0) seleccion = 2;
			else seleccion--;
			vTaskDelay(pdMS_TO_TICKS(300));
		}

		if(joystick.boton == 0) {
			hagl_color_t colorImprimir = hagl_color(display, color.r, color.g, color.b);
			printf("%x\n", colorImprimir);
			while((joystick.boton = gpio_get_level(JOYSTICK_PIN_BUTTON)) == 0) {
				vTaskDelay(pdMS_TO_TICKS(40));
			}
		}
	
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void app_main() {
	configurarPines();

	display = hagl_init();

	wchar_t texto[30];
	wchar_t texto2[30];

	xTaskCreate(input, "Input", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL);

	while(true) {
		hagl_color_t colorPantalla = hagl_color(display, color.r, color.g, color.b);
		swprintf(texto, 30, u"r:%i g:%i b:%i", color.r, color.g, color.b);
		swprintf(texto2, 30, u"seleccion: %c", selecciones[seleccion]);

		hagl_clear(display);

		hagl_fill_rectangle_xyxy(display, 0, 0, display -> width, display -> height, colorPantalla);
		hagl_put_text(display, texto, (display -> width / 2) - 54, (display -> height / 2) - 9, 0xffff ,font6x9);
		hagl_put_text(display, texto2, (display -> width / 2) - 54, (display -> height / 2), 0xffff ,font6x9);

		hagl_flush(display);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
