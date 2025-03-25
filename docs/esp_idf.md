# ESP e IDF
Los ESP32 es una familia de microcontroladores producidas por espressif, misma que desarrolla el software IDF, que es una plataforma de desarrollo para estos y más controladores.

Hay que aclarar que IDF utiliza librerías completamente diferentes a las de Arduino y otros entornos de desarrollo para micontroladores, por lo que, en la mayor parte del tiempo sus librerías y códigos no son compatibles.

Para instalar el entorno de desarrollo se puede hacer de 3 formas.

## Vscode

En vscode hay una extensión que te hace la instalación de todas las herramientas de desarrollo de manera automática

[Enlace](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/get-started/vscode-setup.html)

## Espressif IDE

Su propio entorno de desarrollo, sinceramente poco recomendado debido a que utiliza mucha memoria RAM y posee demasiados errores, sin embargo, es la manera más sencilla de inicializar un proyecto ya que posee las herramientas muy claras.

Para instalarlo hay que marcar la casilla cuando se esten instalando las herramientas de desarrollo:

## idf.py

Es la herramienta de terminal para poder compilar el proyecto en cualquier sistema operativo, en windows tiene su propio instalador (en donde se puede elegir si instalar el IDE o no), en linux hay que compilarlo desde el source code, o si utilizas arch se encuentra en el AUR.

| General | Windows | Aur |
| --- | --- | --- |
| [Enlace](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/get-started/index.html#step-2-get-esp-idf) | [Enlace](https://dl.espressif.com/dl/esp-idf/) | [Enlace](https://aur.archlinux.org/packages/esp-idf) |

> Tener en cuenta que la primera compilación del proyecto siempre es más tardada debido a que tiene que compilar muchas librerías de primera mano, apartir de la segunda compilación se acelera el proceso.

[Documentación oficial](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/api-reference/index.html)