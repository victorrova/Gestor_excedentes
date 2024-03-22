GESTOR EXCEDENTE FOTOVOLTAICO:
==============================

 UN POCO DE HISTORIA:
 ====================
Un buen amigo me mostró un "gestor de excedentes" que compró en Ebay, se trataba de un dimmer con un triac inmenso soldado con 4 cables a una pcb llena de soldadura.
El equipo funcionaba relativamente bien; tenía un firmware llamado Freeds, que hacia su función.

Después de tres modelos les  traigo un  "Frankestein" que hará las delicicias de cualquier Maker con campo Fotovoltaico en su tejado, este amigo, te da la posibilidad de redirigir tu excedentes de produccion  a cualquier carga (regulable claro está).

Se trata de un regulador de potencia (dimmer) de 2Kw con los siguientes atributos:

    - control de potencia mediante Triac:
    =====================================
    No hay nada mejor para controlar potencia que un buen Triac.

    - protección por temperaturra excesiva:
    =======================================
    Incluye una sonda NTC y un ventilador para el controlar la óptima refrigeración.

    - medición de consumo:    
    ======================
    Le he añadido el ic Hlw8032 para la medición ded comsumo (optoaislado por si acaso)

    - Cruce por Cero:
    =================
    Indispensable para un buen accionamiento de Triac.

    - Red Snubber en la salida:
    ===========================
    Por si algún aventurero quiere conectar cargas inductivas....

    - comunicaciones Wifi y bluetooth:
    ==================================
    Todo el sistema esta controlado y comandado por el Maravilloso esp32 de los amigos de Espressif.

ESQUEMA:
========
![esquema](https://github.com/victorrova/librerias/blob/60fdc79c3b665ba7ccb75779944cb327731e6d5e/imagenes/Schematic_gestor.png)

IMAGENES:
=========
![imagen frontal](https://github.com/victorrova/librerias/blob/60fdc79c3b665ba7ccb75779944cb327731e6d5e/imagenes/IMG_20240121_105323.jpg)
![imagen trasera](https://github.com/victorrova/librerias/blob/60fdc79c3b665ba7ccb75779944cb327731e6d5e/imagenes/IMG_20240103_132508_1.jpg)


TRABAJOS PENDIENTES:
====================
Estoy ultimando el firmware de este "bicho", me falta documentarlo todo; creanme cuando vean mi código que les hará falta documentación....
También está en construcción la web para la configuración, y la reparación del meter, que no mide la intensidad como debería.



