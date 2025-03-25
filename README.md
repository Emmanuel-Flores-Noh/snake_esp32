# Snake esp32
Juego de [snake](https://es.wikipedia.org/wiki/La_serpiente_(videojuego)) hecho con las [herramientas de desarrollo de Espressif](https://idf.espressif.com/) y la libería para gráficos [hagl](https://github.com/tuupola/hagl/tree/7d88ff0689a7c726f0ebfd9e019a80903a0ad71c).

## Hardware requerido
| Componente | Enlace de compra |
| ---------- | ---------------- |
| ESP32 USB-c CP2102 | [Enlace](https://es.aliexpress.com/item/1005007059758349.html) |
| Pantalla LCD TFT de 128x160 con el driver ST7735| [Enlace](https://es.aliexpress.com/item/1005006139989470.html) |
| Modulo joystick génerico (Puede ser el del enlace u otro, <br>funcionan de igual forma)| [Enlace](https://es.aliexpress.com/item/1005006195988088.html) |
| Un interruptor de botón (De 2 o 4 pines) |[Enlace](https://es.aliexpress.com/item/1005007623070623.html) |

Recomendación de usar una protoboard doble, pero se puede hacer funcionar con [jumpers](https://es.aliexpress.com/item/1005007298861842.html).

Se requiere soldar el [pin header](https://es.wikipedia.org/wiki/Conector_Berg) de la pantalla.

## Montaje
### Pantalla

| Pin pantalla | Pin ESP32 |
| --- | --------- |
| GND | Tierra |
| VCC | Voltaje de 3v |
| SCL | Pin 18 |
| SDA | Pin 23 |
| RES | Pin 19 |
| DC | pin 22 |
| CS | Pin 21 |
| BL | Voltaje de 3v |

![Montaje pantalla imagen](./docs/src/pantalla%20pinout.png)

### Botón
idk está por verse

### Joystick
idk está por verse

## Instalación
Para clonar el repositorio y las librerías necesarias para compilar el proyecto se puede hacer con el siguiente comando.

```bash
git clone --recursive https://github.com/Emmanuel-Flores-Noh/snake_esp32.git
```

Se utilizó la versión [5.4 de idf](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/get-started/index.html), es recomendable usar esa versión para evitar fallos.

Para compilar el proyecto:
```bash
idf.py build
idf.py flash
```

Si se requiere hacer debugging:
```bash
idf.py flash monitor
```
Si ya se tiene flasheado el programa
```bash
idf.py monitor
```

## Documentación

* [ESP32 e IDF](./docs/esp_idf.md)
* [FreeRTOS](./docs/FreeRTOS.md)
* [hagl](https://github.com/tuupola/hagl)
* [Funcionamiento de los componentes](./docs/funcionamientoComponentes.md)
* funcionamiento del programa (wip)