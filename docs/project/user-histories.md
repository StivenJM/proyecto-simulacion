# Historias de Usuario

Estas historias describen las necesidades principales de las personas que usarán o evaluarán la aplicación. Están escritas desde una perspectiva funcional y académica, sin entrar en detalles técnicos de implementación.

## Usuario Principal

El usuario principal es un estudiante que necesita simular reflexiones difusas en una sala 3D y obtener resultados que respalden un informe académico.

## Historias

### HU-01: Reconocer las superficies de la sala

Como estudiante, quiero que la aplicación identifique los triángulos de la sala, para poder analizar cómo cada superficie participa en la distribución de energía.

Criterios de aceptación:

- La aplicación considera todos los triángulos de la sala.
- Cada triángulo puede participar en los cálculos de la simulación.

### HU-02: Conocer el centro de cada superficie

Como estudiante, quiero que la aplicación calcule el centro de cada triángulo, para usar esos puntos como referencia en el análisis de distancias y desplazamientos de energía.

Criterios de aceptación:

- Cada triángulo tiene un centro calculado.
- El centro calculado se utiliza como referencia para los cálculos entre superficies.

### HU-03: Comparar distancias entre superficies

Como estudiante, quiero conocer la distancia entre los centros de los triángulos, para entender qué tan separadas están las superficies dentro de la sala.

Criterios de aceptación:

- La aplicación calcula distancias entre los triángulos considerados.
- Las distancias se pueden usar para estimar tiempos de desplazamiento.

### HU-04: Identificar superficies visibles entre sí

Como estudiante, quiero que la aplicación determine qué triángulos pueden intercambiar energía, para evitar distribuir energía hacia superficies que no aplican según la consigna.

Criterios de aceptación:

- La aplicación distingue entre triángulos válidos y no válidos para el intercambio de energía.
- Los triángulos del mismo plano no se consideran destinos válidos para la difusión entre ellos.

### HU-05: Calcular tiempos de vuelo

Como estudiante, quiero conocer cuánto tarda la energía en viajar entre superficies visibles, para analizar la evolución temporal de la simulación.

Criterios de aceptación:

- Los tiempos se expresan en milisegundos.
- Los tiempos consideran la velocidad de transmisión definida para el proyecto.
- Los tiempos se usan para ubicar la energía en el momento correspondiente.

### HU-06: Distribuir porcentajes de energía

Como estudiante, quiero que la aplicación calcule el porcentaje de energía enviado a cada superficie visible, para representar una distribución difusa coherente.

Criterios de aceptación:

- Cada superficie visible recibe un porcentaje de energía desde el origen correspondiente.
- Las superficies no visibles no reciben porcentaje desde ese origen.
- La distribución permite repartir la energía entre todos los destinos válidos.

### HU-07: Simular la transición de energía difusa

Como estudiante, quiero que la aplicación actualice la energía de la sala durante la simulación, para observar cómo se desplaza la energía difusa en el espacio y en el tiempo.

Criterios de aceptación:

- La energía se transfiere de acuerdo con los tiempos calculados.
- La energía se reparte de acuerdo con los porcentajes definidos.
- La simulación permite analizar la evolución de la energía durante el intervalo permitido.

### HU-08: Considerar pérdidas por absorción

Como estudiante, quiero que la aplicación considere la absorción de energía, para que la simulación refleje la pérdida indicada por la consigna.

Criterios de aceptación:

- La energía transmitida disminuye según el coeficiente de absorción.
- La energía residual continúa disponible para nuevas transmisiones difusas.
- La pérdida aplicada respeta las condiciones del proyecto.

### HU-09: Analizar energía recibida por receptores

Como estudiante, quiero que la aplicación considere la energía que llega a los receptores, para evaluar el comportamiento acústico desde puntos de recepción dentro de la sala.

Criterios de aceptación:

- La aplicación calcula la energía enviada hacia receptores.
- La energía hacia receptores respeta el tiempo de vuelo correspondiente.
- Los resultados permiten analizar la energía recibida durante la simulación.

### HU-10: Respetar el límite de duración

Como estudiante, quiero que la simulación no supere un segundo, para cumplir con la condición temporal definida en el proyecto.

Criterios de aceptación:

- La simulación limita las transiciones al intervalo permitido.
- No se consideran resultados fuera del límite temporal.

### HU-11: Obtener resultados para el informe

Como estudiante, quiero obtener resultados claros del procedimiento, para elaborar el informe académico con los principales hallazgos.

Criterios de aceptación:

- Los resultados permiten explicar la distribución de energía difusa.
- Los resultados permiten comparar superficies, tiempos y energía recibida.
- La información obtenida sirve como base para el informe solicitado.

