# FreeRTOS

FreeRTOS es un sistema operativo para dispositivos embebidos, su principal función es la de permitir las tareas asincronas con uno o dos nucleos en el procesador.

La idea es que al alternar tan rapidamente entre tareas permite que se pueda "simular" la asincronia con un solo nucleo, lo que nos permite tener tareas demandantes y no demandantes corriendo a la vez sin bloquear el nucleo con una sola de esas tareas.

Esto se puede lograr por diferentes metodos
## Retrasos (delays)
las tareas paran su ejecución por una cantidad de tiempo determinada y con ese tiempo otras tareas empiezan a ejecutarse hasta que pasa la cantidad de tiempo.

## Mutex (Semáforos)
Los mutex son una manera de controlar el flujo de un programa, básicamente, empieza con el estado "Libre" y cuando llega un determinado momento una tarea agarra el nucleo del procesador y pone el semaforo como "Ocupado", así, ninguna otra tarea puede utilizar ese nucleo del procesador hasta que la tarea que la haya agarrado la libere.

## Interrupciones
El programa corre con normalidad, hasta que en algún momento ocurre una tarea con mayor prioridad y se ejecuta, interrumpiendo la función original.
![Interrupts FreeRTOS](https://www.freertos.org/media/2018/rtos_deferred_interrupt_processing.jpg)


[Recomiendo leer la documentación oficial para más detalles](https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/00-Overview)