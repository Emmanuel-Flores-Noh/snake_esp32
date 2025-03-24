#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <hagl_hal.h>
#include <hagl.h>
#include "sdkconfig.h"
#include <wchar.h>

// Fuente
#include <font6x9.h>

// Colores
#define blanco 0xffff
#define cyan 0x9c3d

/* Nada mas por referencia...
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   21
#define PIN_NUM_DC   22
#define PIN_NUM_RST  19
*/

static hagl_backend_t *display;

struct {
	int16_t x, y, dx, dy;
} ball = {};


void physics() {
	ball.x += ball.dx;
	ball.y += ball.dy;
	if(ball.x >= display->width || ball.x <= 0) {
		ball.dx *= -1;
	}
	if(ball.y >= display->height || ball.y <= 0) {
		ball.dy *= -1;
	}
}

void app_main(void) {
	display = hagl_init();

	ball.x = display->width / 2;
	ball.y = display->height / 2;
	ball.dx = 5;
	ball.dy = 5;

	wchar_t texto[12];
	uint8_t textoX, textoY;
	textoX = (display -> width / 2) - (6 * 3);
	textoY = (display -> height / 2) - 9;

	while(true) {
		hagl_clear(display);
		swprintf(texto, 12, u"%i,%i", ball.x, ball.y);
		hagl_put_text(display, texto, textoX, textoY, cyan, font6x9);
		hagl_fill_circle(display, ball.x, ball.y, 10, blanco);
		physics();
		hagl_flush(display);
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	// hagl_fill_rectangle(display, 128 / 3, 0, (128 / 3) * 2, 160, 0xffff);
}
