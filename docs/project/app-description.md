# Simulador Acústico en una Sala 3D

La aplicación permite crear, visualizar y simular escenarios acústicos dentro de una sala 3D. Su objetivo es apoyar el estudio del comportamiento de la energía sonora mediante el análisis de rayos de reflexión y la posterior distribución de energía difusa entre superficies y receptores.

## Propósito

El proyecto busca representar un escenario de simulación acústica donde el usuario pueda preparar la sala, ubicar fuentes y receptores, observar los rayos de reflexión y analizar cómo la energía se redistribuye de forma difusa. A partir de la información del escenario, la aplicación debe estimar qué superficies participan en el intercambio de energía, cuánto tarda esa energía en desplazarse y qué porcentaje corresponde a cada destino visible.

## Problema que resuelve

En una sala cerrada, la energía acústica puede viajar mediante rayos de reflexión y también dispersarse de manera difusa entre superficies y receptores. Para estudiar este fenómeno, se necesita una herramienta que permita construir escenarios, visualizar el comportamiento de los rayos y organizar la evolución de la energía en el espacio y en el tiempo.

## Alcance de la aplicación

La aplicación debe permitir:

- Crear y editar escenarios simples dentro de la aplicación.
- Ubicar fuentes de energía y receptores en la sala.
- Definir la energía inicial de cada fuente.
- Configurar la cantidad global de rayos usada para muestrear direcciones de simulación.
- Visualizar el escenario de forma interactiva en 3D.
- Observar la simulación desde una vista externa y desde una vista interna tipo cámara.
- Analizar rayos de reflexión dentro del escenario.
- Identificar el punto central de cada superficie triangular de la sala.
- Estimar la distancia entre superficies de la sala.
- Calcular el tiempo aproximado que tomaría la energía en desplazarse entre superficies visibles.
- Determinar qué porcentaje de energía se distribuye hacia cada superficie visible.
- Configurar coeficientes de absorción para las superficies del escenario.
- Actualizar la energía de la sala de acuerdo con los tiempos, porcentajes y pérdidas por absorción.
- Considerar la energía que finalmente llega a los receptores.
- Guardar y compartir archivos de escenario.
- Exportar resultados y visualizaciones de la simulación.
- Mantener la simulación dentro de una duración máxima de un segundo.
- Generar información útil para elaborar el informe académico del proyecto.

## Resultado esperado

Al finalizar, la aplicación debe ofrecer una simulación funcional del comportamiento acústico en una sala 3D, incluyendo rayos de reflexión, redistribución de energía difusa y resultados observables desde el escenario. El usuario puede preparar fuentes con energía configurable, ajustar la cantidad global de rayos y revisar la reproducción de la simulación a distintas velocidades. El resultado debe servir para analizar los principales hallazgos del procedimiento y respaldar el informe académico solicitado para la evaluación.

## Criterio de éxito

El proyecto se considera exitoso si la aplicación permite preparar un escenario acústico, visualizar la simulación, representar rayos de reflexión, distribuir energía difusa entre superficies visibles y receptores, respetar las condiciones indicadas en la consigna y obtener resultados claros para su análisis académico.
