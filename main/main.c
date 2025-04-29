#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <hagl.h>
#include <hagl_hal.h>
#include <hagl_hal_color.h>

#include "sdkconfig.h" 
#include <wchar.h>
#include <hal/adc_types.h>

#include "esp_random.h"

// Fuente
#include <font6x9.h>

// Sprites
#include "cabeza.h"
#include "manzana.h"
#include "cola.h"
#include "Cuerpo.h"
#include "Giro.h"

struct {hagl_bitmap_t arriba, abajo, izquierda, derecha;} bitmapsCabeza;
struct {hagl_bitmap_t arriba, abajo, izquierda, derecha;} bitmapsCola;
struct {hagl_bitmap_t x, y, izqArr, derArr, izqAba, derAba; } bitmapsCuerpo;
hagl_bitmap_t bitmapManzana;
hagl_color_t bitmapTemporalArray[512];

// Colores
#include "colores.h"

// Pines del joystick Pins 34 y 35
#define JOYSTICK_PIN_X ADC1_CHANNEL_6
#define JOYSTICK_PIN_Y ADC1_CHANNEL_7
#define JOYSTICK_PIN_BUTTON GPIO_NUM_13
SemaphoreHandle_t xDrawHandle;

// Algunas configuraciones
#define inputBufferSize 5
void generarFruta();

// Display
static hagl_backend_t *display;

// Estados del juego
enum estadosJuego {
	MENU,
	JUGANDO,
	PAUSE,
	GAMEOVER,
	GANAR
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

uint8_t contadorSecciones;

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


// Dificultades
unsigned int velocidades[3];
enum dificultad {
	FACIL = 0,
	NORMAL = 1,
	DIFICIL = 2
};
char *velocidadesLabel[] = {"FACIL", "NORMAL", "DIFICIL"};
enum dificultad seleccionVelocidad = NORMAL;

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
	
	gpio_config(&joybutton); }

hagl_color_t* rotarBitmap(const hagl_color_t *bitmap, enum direcciones rotacion) {
	hagl_color_t *salida = malloc(256 * sizeof(hagl_color_t));
	int contador;
	switch(rotacion) {
		case ARRIBA:
			for(int i = 0; i < 256; i++) {
				salida[i] = bitmap[i];
			}
			break;
		case ABAJO:
			for(int i = 0; i < 256; i++) {
				salida[255 - i] = bitmap[i];
			}
			break;
		case IZQUIERDA:
			contador = 15;
			for(int i = 0; i < 256; i++) {
				salida[i] = bitmap[contador % 256];
				contador += 16;
				if(i > 0 && (i+1) % 16 == 0) {
					contador--;
				}
			}
			break;
		case DERECHA:
			contador = 240;
			for(int i = 0; i < 256; i++) {
				salida[i] = bitmap[contador % 256];
				contador += 240;
				if(i > 0 && (i+1) % 16 == 0) {
					contador++;
				}
			}
			break;
	}
	return salida;
}

int8_t blink = 1;

void blink_change(void *params) {
	while(true) {
		blink *= -1;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// Funcion para renderizar en la pantalla
void IRAM_ATTR renderizar(void *params) {
	wchar_t texto[50];

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(true) {
		if(! xSemaphoreTake(xDrawHandle, portMAX_DELAY) ) {
			continue;
		}
		hagl_clear(display);

		if(estadoJuego == MENU) {
			hagl_put_text(display, (wchar_t *)u"SNAKE ESP32", display -> width / 2 - 30, display -> height / 2 - 18, BLANCO, font6x9);
			hagl_put_text(display, texto, display -> width / 2 - 50, display -> height / 2, BLANCO, font6x9);
			swprintf(texto, 50, (wchar_t *)u"Dificultad: %s", velocidadesLabel[seleccionVelocidad]);
			if(blink == 1) {
				hagl_put_text(display, (wchar_t *)u"Presiona para iniciar", display -> width / 2 - 60, display -> height / 2 + 18, BLANCO, font6x9);
			}
		}
		else if(estadoJuego == JUGANDO || estadoJuego == PAUSE) {
			hagl_fill_rectangle(display, 0, 0, display -> width, display -> height, FONDO);

			hagl_bitmap_t *sprite = NULL;

			// Dibujar cabeza
			switch(direccion) {
				case ARRIBA:
					sprite = &bitmapsCabeza.arriba;
					break;
				case ABAJO:
					sprite = &bitmapsCabeza.abajo;
					break;
				case IZQUIERDA:
					sprite = &bitmapsCabeza.izquierda;
					break;
				case DERECHA:
					sprite = &bitmapsCabeza.derecha;
					break;
			}
			hagl_blit(display, serpiente[0].x * 16, serpiente[0].y * 16, sprite);

			for(uint8_t i = 1; i < contadorSecciones - 1; i++) {
				// hagl_fill_rectangle(display, serpiente[i].x * 16, serpiente[i].y * 16, (serpiente[i].x + 1) * 16, (serpiente[i].y + 1) * 16, CYAN);

				sprite = NULL;

				coords *actual = &serpiente[i];
				coords *anterior = &serpiente[i+1];
				coords *siguiente = &serpiente[i-1];

				// Si la serpiente estaba yendo verticalmente
				if(actual -> x == anterior -> x) {
					if(actual -> x == siguiente -> x) {
						sprite = &bitmapsCuerpo.y;
					}
					// Si el siguiente esta a la derecha
					else if(actual -> x < siguiente -> x) {
						// Si estaba yendo hacia arriba
						if(actual -> y < anterior -> y) {
							sprite = &bitmapsCuerpo.derAba;
						}
						// Si estaba yendo hacia abajo
						else {
							sprite = &bitmapsCuerpo.derArr;
						}
					}
					// Si el siguiente esta a la izquierda
					else {
						// Si estaba yendo hacia arriba
						if(actual -> y < anterior -> y) {
							sprite = &bitmapsCuerpo.izqAba;
						}
						// Si estaba yendo hacia abajo
						else {
							sprite = &bitmapsCuerpo.izqArr;
						}
					}
				}
				// Si la serpiente estaba yendo horizontalmente
				else {
					if(actual -> y == siguiente -> y) {
						sprite = &bitmapsCuerpo.x;
					}
					// Si el siguiente esta arriba
					else if(actual -> y > siguiente -> y) {
						// Si estaba yendo hacia derecha
						if(actual -> x < anterior -> x) {
							sprite = &bitmapsCuerpo.derArr;
						}
						// Si estaba yendo hacia izquierda
						else {
							sprite = &bitmapsCuerpo.izqArr;
						}
					}
					// Si el siguiente esta abajo
					else {
						// Si estaba yendo hacia derecha
						if(actual -> x < anterior -> x) {
							sprite = &bitmapsCuerpo.derAba;
						}
						// Si estaba yendo hacia izquierda
						else {
							sprite = &bitmapsCuerpo.izqAba;
						}
					}
				}

				// Corregir cuando la serpiente da la vuelta en el modo facil
				if(seleccionVelocidad == FACIL) {
					if(abs(actual -> x - anterior -> x) > 1 || abs(actual -> x - siguiente -> x) > 1) {
						if(sprite == &bitmapsCuerpo.derArr) {
							sprite = &bitmapsCuerpo.izqArr;
						}
						else if(sprite == &bitmapsCuerpo.izqArr) {
							sprite = &bitmapsCuerpo.derArr;
						}
						else if(sprite == &bitmapsCuerpo.izqAba) {
							sprite = &bitmapsCuerpo.derAba;
						}
						else if(sprite == &bitmapsCuerpo.derAba) {
							sprite = &bitmapsCuerpo.izqAba;
						}
					}
					if(abs(actual -> y - anterior -> y) > 1 || abs(actual -> y - siguiente -> y) > 1) {
						if(sprite == &bitmapsCuerpo.derArr) {
							sprite = &bitmapsCuerpo.derAba;
						}
						else if(sprite == &bitmapsCuerpo.izqArr) {
							sprite = &bitmapsCuerpo.izqAba;
						}
						else if(sprite == &bitmapsCuerpo.izqAba) {
							sprite = &bitmapsCuerpo.izqArr;
						}
						else if(sprite == &bitmapsCuerpo.derAba) {
							sprite = &bitmapsCuerpo.derArr;
						}
					}
				}

				hagl_blit(display, serpiente[i].x * 16, serpiente[i].y * 16, sprite);
			}

			if(serpiente[contadorSecciones - 1].x < serpiente[contadorSecciones - 2].x) {
				sprite = &bitmapsCola.derecha;
			}
			else if(serpiente[contadorSecciones - 1].x > serpiente[contadorSecciones - 2].x) {
				sprite = &bitmapsCola.izquierda;
			}
			else if(serpiente[contadorSecciones - 1].y > serpiente[contadorSecciones - 2].y) {
				sprite = &bitmapsCola.arriba;
			}
			else {
				sprite = &bitmapsCola.abajo;
			}
			
			// Corregir la cola en el modo facil
			if(seleccionVelocidad == FACIL) {
				if( abs(serpiente[contadorSecciones - 1].x - serpiente[contadorSecciones - 2].x) > 1) {
					if(sprite == &bitmapsCola.izquierda) {
						sprite = &bitmapsCola.derecha;
					}
					else {
						sprite = &bitmapsCola.izquierda;
					}
				}
				else if(abs(serpiente[contadorSecciones - 1].y - serpiente[contadorSecciones - 2].y) > 1) {
					if(sprite == &bitmapsCola.abajo) {
						sprite = &bitmapsCola.arriba;
					}
					else {
						sprite = &bitmapsCola.abajo;
					}
				}
			}

			hagl_blit(display, serpiente[contadorSecciones - 1].x * 16, serpiente[contadorSecciones - 1].y * 16, sprite);

			// fruta beta
			// hagl_fill_rectangle(display, fruta.x * 16, fruta.y * 16, (fruta.x + 1) * 16, (fruta.y + 1) * 16, NEGRO);

			hagl_blit(display, fruta.x * 16, fruta.y * 16, &bitmapManzana);

			if(estadoJuego == PAUSE) {
				hagl_put_text(display, (wchar_t *)u"PAUSE", display -> width / 2 - 15, display -> height / 2 - 9, 
				blink == 1 ? BLANCO : NEGRO, font6x9);
			}
		}
		else if(estadoJuego == GAMEOVER) {
			hagl_put_text(display, (wchar_t *)u"GAMEOVER", display -> width / 2 - 30, display -> height / 2 - 18, BLANCO, font6x9);

			swprintf(texto, 50, (wchar_t *)u"puntuacion: %i", contadorSecciones);
			hagl_put_text(display, texto, display -> width / 2 - 30, display -> height / 2, BLANCO, font6x9);
		}
		else if(estadoJuego == GANAR) {
			hagl_put_text(display, (wchar_t *)u"GANASTE", display -> width / 2 - 30, display -> height / 2 - 18, BLANCO, font6x9);
		}
		hagl_flush(display);
		xSemaphoreGive(xDrawHandle);
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
	}
}

struct {
	uint16_t x,y;
} joystick;

uint8_t boton;

enum direcciones inputBuffer[inputBufferSize];

void IRAM_ATTR input(void *param) {
	while(true) {
		joystick.x = adc1_get_raw(JOYSTICK_PIN_X);
		joystick.y = adc1_get_raw(JOYSTICK_PIN_Y);
		boton = gpio_get_level(JOYSTICK_PIN_BUTTON);
		// printf("%i %i %i\n", joystick.x, joystick.y, boton);

		if(estadoJuego == MENU) {
			// generarFruta();
			if(boton == 0) {
				fruta.x = 6 + esp_random() % 3;
				fruta.y = esp_random() % tableroAlto;
	
				contadorSecciones = 3;
				direccion = DERECHA;
				for(int i = 0; i < tableroAlto * tableroAncho; i++) {
					serpiente[i].x = 0; 
					serpiente[i].y = 0;
				}
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
	
				serpiente[0].x = 3;
				serpiente[0].y = 4;
				serpiente[1].x = 2;
				serpiente[1].y = 4;
				serpiente[2].x = 1;
				serpiente[2].y = 4;
				estadoJuego = JUGANDO;
			}
			else if(joystick.y <= 0) {
				while((joystick.y = adc1_get_raw(JOYSTICK_PIN_Y)) <= 0) {
					seleccionVelocidad = (seleccionVelocidad + 1) % 3;
					printf("%i\n", seleccionVelocidad);
					vTaskDelay(pdMS_TO_TICKS(300));
				}
			}
			else if(joystick.y >= 4095) {
				while((joystick.y = adc1_get_raw(JOYSTICK_PIN_Y)) >= 4095) {
					if(seleccionVelocidad == FACIL) {
						seleccionVelocidad = DIFICIL;
					}
					else {
						seleccionVelocidad--;
					}
					printf("%i\n", seleccionVelocidad);
					vTaskDelay(pdMS_TO_TICKS(300));
				}

			}
		}
		else if(estadoJuego == JUGANDO) {
			if(boton == 0) {
				estadoJuego = PAUSE;
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
				continue;
			}

			int8_t inputIndex = -1;


			// Comprueba si se va a guardar el input
			for(int i = 0; i < inputBufferSize; i++) {
				if(inputBuffer[i] != -1) {
					continue;
				}
				inputIndex = i;
				break;
			}

			if(inputIndex == -1) {
				// No se pueden guardar mas inputBuffers
				vTaskDelay(pdMS_TO_TICKS(150));
				continue;
			}

			enum direcciones *buffer = &inputBuffer[inputIndex];

			switch(direccion) {
				case ARRIBA:
				case ABAJO:
					if(inputIndex == 0 || ( inputBuffer[inputIndex - 1] != IZQUIERDA && inputBuffer[inputIndex - 1] != DERECHA )) {
						if(joystick.x <= 0) {
							printf("Asignado izquierda\n");
							*buffer = IZQUIERDA;
						}
						else if(joystick.x >= 4095) {
							printf("Asignado derecha\n");
							*buffer = DERECHA;
						}
					}
					break;
				case IZQUIERDA:
				case DERECHA:
					if(inputIndex == 0 || ( inputBuffer[inputIndex - 1] != ARRIBA && inputBuffer[inputIndex - 1] != ABAJO )) {
						if(joystick.y <= 0) {
							printf("Asignado arriba\n");
							*buffer = ARRIBA;
						}
						else if(joystick.y >= 4095) {
							printf("Asignado abajo\n");
							*buffer = ABAJO;
						}
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
		else if(estadoJuego == GAMEOVER) {
			if(boton == 0) {
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
				estadoJuego = MENU;
			}
		}
		else if(estadoJuego == GANAR) {	
			if(boton == 0) {
				while(gpio_get_level(JOYSTICK_PIN_BUTTON) == 0) {vTaskDelay(pdMS_TO_TICKS(300));}
				estadoJuego = MENU;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

coords ultimaPos;

void movimientoSerpiente() {
	if(! xSemaphoreTake(xDrawHandle, portMAX_DELAY)) {
		return;
	}
	ultimaPos.x = serpiente[contadorSecciones - 1].x;
	ultimaPos.y = serpiente[contadorSecciones - 1].y;
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
	
	if(seleccionVelocidad == FACIL) {
		if(serpiente[0].x < 0) {
			serpiente[0].x = tableroAncho - 1;
		}
		else if(serpiente[0].x > tableroAncho - 1) {
			serpiente[0].x = 0;
		}
		else if(serpiente[0].y < 0) {
			serpiente[0].y = tableroAlto - 1;
		}
		else if(serpiente[0].y > tableroAlto - 1) {
			serpiente[0].y = 0;
		}
	}

	xSemaphoreGive(xDrawHandle);
}

int8_t buscarEnSerpiente(coords buscar) {
	for(int i = 0; i < contadorSecciones; i++) {
		if(serpiente[i].x == buscar.x && buscar.y == serpiente[i].y) {
			return 1;
		}
	}
	return 0;
}

void generarFruta() {
	coords libres[80 - contadorSecciones];
	int8_t contadorLibres = 0;
	coords busquedaActual = {
		.x = 0,
		.y = 0,
	};
	int8_t seccionesEncontradas = 0;
	for(int i = 0; i < 80; i++) {
		if(seccionesEncontradas < contadorSecciones && buscarEnSerpiente(busquedaActual) == 1){
			seccionesEncontradas++;
		}
		else {
			libres[contadorLibres].x = busquedaActual.x;
			libres[contadorLibres].y = busquedaActual.y;
			contadorLibres++;
		};
		busquedaActual.x++;
		if(busquedaActual.x >= tableroAncho) {
			busquedaActual.x = 0;
			busquedaActual.y++;
		}
	}

	int rng = esp_random() % (80 - contadorSecciones);
	fruta.x = libres[rng].x;
	fruta.y = libres[rng].y;
}

void comprobarComer() {
	if(fruta.x == serpiente[0].x && fruta.y == serpiente[0].y) {
		contadorSecciones++;
		if(contadorSecciones == tableroAlto * tableroAncho) {
			estadoJuego = GANAR;
			return;
		}
		serpiente[contadorSecciones - 1] = ultimaPos;
		generarFruta();
	}
}

uint8_t comprobarMuerte() {
	coords *cabeza = &serpiente[0];

	for(int i = 1; i < contadorSecciones; i++) {
		if(cabeza -> x == serpiente[i].x && cabeza -> y == serpiente[i].y) {
			return 1;
		}
	}

	if(cabeza -> x < 0 || cabeza -> x > tableroAncho - 1 || cabeza -> y < 0 || cabeza -> y > tableroAlto - 1) {
		return 1;
	}

	return 0;
}

void app_main(void)
{
	// Inicializar mutex
	xDrawHandle = xSemaphoreCreateMutex();

	// Configurar joystick y botones
	configurarPines();
	display = hagl_init();

	// Inicializar las velocidades
	velocidades[0] = pdMS_TO_TICKS(1000);
	velocidades[1] = pdMS_TO_TICKS(1000);
	velocidades[2] = pdMS_TO_TICKS(500);

	// inicializar el InputBuffer
	for(int i = 0; i < inputBufferSize; i++) {
		inputBuffer[i] = -1;
	}
	
	// Inicializar sprite de la manzanita
	hagl_bitmap_init(&bitmapManzana, 16, 16, 1, bitmapManzanaArray);

	// Inicializar sprites de la cabeza
	hagl_bitmap_init(&bitmapsCabeza.arriba, 16, 16, 1, bitmapCabezaArray);

	hagl_bitmap_init(&bitmapsCabeza.abajo, 16, 16, 1, rotarBitmap(bitmapCabezaArray, ABAJO));
	hagl_bitmap_init(&bitmapsCabeza.izquierda, 16, 16, 1, rotarBitmap(bitmapCabezaArray, IZQUIERDA));
	hagl_bitmap_init(&bitmapsCabeza.derecha, 16, 16, 1, rotarBitmap(bitmapCabezaArray, DERECHA));

	// Inicializar sprites de la cola
	hagl_bitmap_init(&bitmapsCola.arriba, 16, 16, 1, bitmapColaArray);
	
	hagl_bitmap_init(&bitmapsCola.abajo, 16, 16, 1, rotarBitmap(bitmapColaArray, ABAJO));
	hagl_bitmap_init(&bitmapsCola.izquierda, 16, 16, 1, rotarBitmap(bitmapColaArray, IZQUIERDA));
	hagl_bitmap_init(&bitmapsCola.derecha, 16, 16, 1, rotarBitmap(bitmapColaArray, DERECHA));

	// Inicializar sprites del cuerpo
	hagl_bitmap_init(&bitmapsCuerpo.y, 16, 16, 1, bitmapCuerpoArray);
	hagl_bitmap_init(&bitmapsCuerpo.x, 16, 16, 1, rotarBitmap(bitmapCuerpoArray, IZQUIERDA));

	hagl_bitmap_init(&bitmapsCuerpo.izqArr, 16, 16, 1, bitmapGiroArray);
	hagl_bitmap_init(&bitmapsCuerpo.derArr, 16, 16, 1, rotarBitmap(bitmapGiroArray, DERECHA));
	hagl_bitmap_init(&bitmapsCuerpo.derAba, 16, 16, 1, rotarBitmap(bitmapGiroArray, ABAJO));
	hagl_bitmap_init(&bitmapsCuerpo.izqAba, 16, 16, 1, rotarBitmap(bitmapGiroArray, IZQUIERDA));

	// Iniciar tarea de renderizado
	xTaskCreatePinnedToCore(renderizar, "Renderizado", 4000, NULL, 4 | portPRIVILEGE_BIT, NULL, 1);
	// Iniciar tarea del input
	xTaskCreatePinnedToCore(input, "Input", 2000, NULL, 1 | portPRIVILEGE_BIT, NULL, 0);
	// Iniciar tarea del blink
	xTaskCreate(blink_change, "Blink", 1500, NULL, 0 | portPRIVILEGE_BIT, NULL);

	estadoJuego = MENU;

	while(true) {
		if(estadoJuego == JUGANDO) {
			movimientoSerpiente();
			comprobarComer();
			if(comprobarMuerte()) {
				estadoJuego = GAMEOVER;
			}
		}
		vTaskDelay(velocidades[seleccionVelocidad]);
	}
}
