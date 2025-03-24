#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <hagl_hal.h>
#include <hagl.h>
#include "sdkconfig.h"

#define WIDTH 128
#define HEIGHT 160

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   21
#define PIN_NUM_DC   22
#define PIN_NUM_RST  19

static hagl_backend_t *display;

struct {
	uint8_t x, y, dx, dy;
} ball = {};


void physics() {
	ball.x += ball.dx;
	ball.y += ball.dy;
	if(ball.x >= WIDTH || ball.x <= 0) {
		ball.dx *= -1;
	}
	if(ball.y >= HEIGHT || ball.y <= 0) {
		ball.dy *= -1;
	}
}

void app_main(void) {
	display = hagl_init();

	ball.x = WIDTH / 2;
	ball.y = HEIGHT / 2;
	ball.dx = 5;
	ball.dy = 5;

	while(true) {
		hagl_clear(display);
		hagl_fill_circle(display, ball.x, ball.y, 10, 0xffff);
		physics();
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	// hagl_fill_rectangle(display, 128 / 3, 0, (128 / 3) * 2, 160, 0xffff);
}
