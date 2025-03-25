# Funcionamiento de los componentes

Los diferentes pines del ESP32 tienen distintos propositos, en general, para lo que nos importa lo podemos dividir en dos secciones
* Digitales: Leen si llega o no voltaje, es decir, son interruptores booleanos, 0 para apagado y 1 para encendido.
* Analogicos: Leen cuanto voltaje llega al pin, es decir, que puede tomar valores que no sean 0 y 1, esto sobretodo sirve para recibir datos de otros perifericos como pueden ser termómetros.

![ESP32 30pins pinout](https://lastminuteengineers.com/wp-content/uploads/iot/ESP32-Pinout.png)

[Para más información acerca de los pines](https://lastminuteengineers.com/esp32-pinout-reference/)

Ahora, los componentes de menos a más complejo...

## Botón
Es el más sencillo, agarra un voltaje de 3v y al pulsarlo deja pasar la corriente a los pines que tenga, ese voltaje lo podemos medir con un pin digital para así registrarlo en el código.

![Boton imagen](https://inputmakers.com/wp-content/uploads/2020/11/pulsador-arduino-1-1024x563.png)

## Joystick
Todos los modulos joystick funcionan igual independientemente de la tecnología que usen para calcular las coordenadas, sea por potenciometros o efecto HALL.

Se conecta a tierra y a una entrada de 5V (Se aclarará esto más tarde), junto con dos pines de salida de datos, VRx para el valor en X y VRy para el valor de Y, y ambas entradas deben ser analogicas, y SW que va conectado a un pin digital y hace de interruptor cuando pulsamos el joystick (es decir, cuando pulsamos L3).

Se menciona que usa una entrada de 5V, pero al parecer con el ESP32 esto da problemas debido a que da señales y lecturas incorrectas, por eso en este caso en vez de 5V se alimenta con 3V.

![Joystick pinout](https://components101.com/sites/default/files/component_pin/Joystick-Module-Pinout.png)

## Display
Sin duda el elemento más complicado de todos. 
Manipularlo por código no es tan complicado de entender y aplicar, sin embargo, su funcionamiento a nivel de electrónica es mucho más complejo, por eso nos quedaremos con las bases.

![pantalla pinout](./src/pantalla%20pinout.png)

La pantalla y el voltaje van conectados a sus respectivas entradas.

El SCL es un pin especial que funciona como un reloj que sincroniza los datos que se envian a la pantalla.

El SDA es el pin que recibe los datos, en este caso el pin a utilizar en el ESP32 debe contar con el protocolo MOSI (Master Output Slave Input) para enviarle información a la pantalla, tambien existe el MISO(Master Input Slave Output) que es el que permite recibir datos pero no es relevante para este proyecto.

El RST es un pin digital que indica a la pantalla si debe borrar los datos que tenga en esta o no (Basicamente, limpiar la pantalla).

El pin DC indica a la pantalla si debe recibir datos para mostrar, o si debe ejecutar un comando que se le envie.

EL pin CS permite seleccionar a cual dispositivo estamos comunicandonos por medio del bus de datos que utilizamos (MOSI).

BL hace referencia al Backlight, es decir, la intensidad del brillo de la pantalla, como no hay necesidad de modificarla en este proyecto puede ir siempre conectada al voltaje de 3v.