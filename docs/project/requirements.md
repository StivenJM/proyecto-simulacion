# Requerimientos del Proyecto

Este documento describe lo que debe cumplir la aplicación desde el punto de vista del proyecto. No define arquitectura ni detalles de programación; describe comportamientos esperados y condiciones de aceptación.

## Requerimientos Funcionales

### RF-01: Identificación de superficies

La aplicación debe identificar todos los triángulos que conforman la sala 3D.

Aceptación:

- Cada triángulo de la sala debe ser considerado en el análisis.
- La aplicación debe poder asociar cálculos posteriores a cada triángulo identificado.

### RF-02: Cálculo del centro de cada triángulo

La aplicación debe determinar el centroide de cada triángulo de la sala.

Aceptación:

- Cada triángulo debe contar con un punto central calculado.
- Ese punto debe usarse como referencia para las relaciones entre superficies.

### RF-03: Cálculo de distancias entre superficies

La aplicación debe calcular la distancia entre los centros de los triángulos de la sala.

Aceptación:

- Debe existir una distancia para cada par de triángulos considerado.
- Las distancias deben permitir comparar qué tan separadas están las superficies entre sí.

### RF-04: Evaluación de visibilidad entre triángulos

La aplicación debe determinar qué triángulos pueden intercambiar energía difusa según la condición de visibilidad definida por el proyecto.

Aceptación:

- Dos triángulos pueden considerarse visibles entre sí cuando no pertenecen al mismo plano.
- Los triángulos que no cumplan esta condición no deben recibir energía difusa entre ellos.

### RF-05: Cálculo de tiempos de vuelo

La aplicación debe calcular el tiempo aproximado que tomaría la energía en desplazarse entre triángulos visibles.

Aceptación:

- El cálculo debe considerar una velocidad de transmisión de 340 m/s.
- El tiempo debe expresarse en milisegundos.
- Los tiempos deben estar disponibles para coordinar la transición de energía.

### RF-06: Cálculo de porcentajes de distribución

La aplicación debe calcular qué porcentaje de energía difusa se enviará desde un triángulo hacia los triángulos visibles.

Aceptación:

- El porcentaje debe considerar la relación geométrica entre la superficie de origen y las superficies visibles de destino.
- La distribución debe permitir repartir la energía entre todos los destinos válidos.
- Los triángulos no visibles no deben recibir porcentaje de energía desde ese origen.

### RF-07: Distribución de energía difusa entre superficies

La aplicación debe distribuir la energía difusa entre los triángulos visibles de la sala.

Aceptación:

- La energía debe desplazarse respetando los tiempos de vuelo calculados.
- La energía enviada debe corresponder a los porcentajes definidos para cada destino.
- La energía debe registrarse de forma que pueda analizarse su evolución en el tiempo.

### RF-08: Aplicación de absorción de energía

La aplicación debe considerar la pérdida de energía causada por el coeficiente de absorción.

Aceptación:

- La energía transmitida debe disminuir según la absorción correspondiente.
- La energía residual debe poder continuar su transmisión difusa.
- No deben aplicarse pérdidas adicionales a la energía residual más allá de lo indicado por la consigna.

### RF-09: Transmisión hacia receptores

La aplicación debe transmitir energía hacia los receptores considerados en la sala.

Aceptación:

- La energía enviada a receptores debe considerar el porcentaje correspondiente.
- El cálculo debe respetar el tiempo de vuelo entre el triángulo de origen y el receptor.
- Los resultados deben permitir analizar la energía recibida durante la simulación.

### RF-10: Límite temporal de simulación

La aplicación debe limitar la transición de energía a una duración máxima de un segundo.

Aceptación:

- No deben registrarse transiciones posteriores al límite definido.
- Los resultados deben corresponder únicamente al intervalo de simulación permitido.

### RF-11: Resultados para análisis académico

La aplicación debe generar información suficiente para describir los principales hallazgos del procedimiento.

Aceptación:

- Los resultados deben permitir explicar cómo se distribuyó la energía difusa.
- Los resultados deben servir como base para el informe académico del proyecto.
- La información obtenida debe ser clara para comparar superficies, tiempos, porcentajes y energía recibida.

### RF-12: Visualización 3D interactiva

La aplicación debe permitir visualizar el escenario de simulación en un entorno 3D interactivo.

Aceptación:

- El usuario debe poder observar el escenario completo de manera visual.
- La visualización debe ayudar a comprender la posición de superficies, fuentes y receptores.
- La interacción debe facilitar la revisión del escenario antes y durante la simulación.

### RF-13: Creación y edición de escenarios

La aplicación debe permitir que el usuario cree y edite escenarios de simulación dentro de la propia aplicación.

Aceptación:

- El usuario debe poder construir escenarios simples usando planos o formas básicas compuestas por planos.
- El usuario debe poder modificar la ubicación y disposición de los elementos del escenario.
- El escenario preparado debe poder usarse posteriormente en la simulación.

### RF-14: Configuración de fuentes y receptores

La aplicación debe permitir colocar y configurar fuentes de energía y receptores dentro del escenario.

Aceptación:

- El usuario debe poder ubicar fuentes de energía en el escenario.
- El usuario debe poder ubicar receptores en el escenario.
- Las fuentes y receptores definidos deben participar en la simulación.

### RF-15: Coeficiente de absorción por superficie

La aplicación debe permitir asignar un coeficiente de absorción distinto a cada plano o superficie del escenario.

Aceptación:

- El usuario debe poder definir la absorción de cada superficie.
- La simulación debe considerar el coeficiente asignado a cada superficie.
- Las diferencias entre superficies deben reflejarse en los resultados de energía.

### RF-16: Modos de preparación y simulación

La aplicación debe tener un modo para preparar los elementos de simulación y otro modo para ejecutar la simulación.

Aceptación:

- En el modo de preparación, el usuario debe poder editar el escenario, fuentes, receptores y propiedades relevantes.
- En el modo de simulación, el usuario debe poder iniciar la simulación preparada.
- La separación entre modos debe ayudar a evitar cambios accidentales durante la ejecución.

### RF-17: Vistas durante la simulación

La aplicación debe ofrecer dos formas de observar la simulación: una vista externa del escenario completo y una vista interna desde la perspectiva de un usuario dentro del escenario.

Aceptación:

- La vista externa debe permitir observar el escenario completo desde fuera.
- La vista interna debe permitir desplazarse dentro del escenario como si el usuario estuviera allí.
- El usuario debe poder iniciar la simulación desde el modo de simulación.

### RF-18: Compartición de escenarios

La aplicación debe permitir compartir archivos de escenario para que otra persona pueda abrir la misma simulación en la aplicación.

Aceptación:

- El escenario debe poder guardarse como archivo.
- El archivo compartido debe conservar la configuración del escenario, superficies, fuentes, receptores y propiedades necesarias.
- Otro usuario debe poder cargar el archivo para revisar o ejecutar la simulación.

### RF-19: Exportación completa de resultados

La aplicación debe permitir exportar los resultados de la simulación junto con información visual útil del escenario.

Aceptación:

- La exportación debe incluir datos relevantes de la simulación, como tiempos, porcentajes y energía recibida.
- La exportación debe incluir información visual o capturas que ayuden a interpretar el escenario y los resultados.
- La información exportada debe ser útil para análisis, presentación y elaboración del informe académico.

## Requerimientos No Funcionales

### RNF-01: Claridad de resultados

Los resultados deben presentarse de manera comprensible para apoyar el análisis del proyecto.

### RNF-02: Coherencia con la consigna

La aplicación debe respetar las condiciones dadas en el enunciado del proyecto.

### RNF-03: Utilidad académica

La información producida debe facilitar la elaboración del informe bajo el formato académico solicitado.

### RNF-04: Trazabilidad del procedimiento

Debe ser posible relacionar los resultados obtenidos con los pasos principales del cálculo de energía difusa.

## Fuera de Alcance

Los siguientes puntos no forman parte del alcance actual del proyecto:

- Simular geometrías complejas o modelos arquitectónicos detallados que excedan escenarios simples creados con planos o formas básicas.
- Importar modelos 3D avanzados desde herramientas externas de modelado.
- Realizar simulaciones físicas distintas a la reflexión difusa solicitada para este proyecto.
- Incluir colaboración en tiempo real entre varios usuarios editando el mismo escenario al mismo tiempo.
- Generar automáticamente el informe académico final completo; la aplicación solo debe entregar resultados e información de apoyo.
- Garantizar precisión profesional para diseño acústico real; el objetivo es académico y está limitado por las condiciones de la consigna.
