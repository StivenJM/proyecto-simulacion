# Guía de uso

Esta guía resume el flujo principal para preparar y ejecutar una simulación acústica 3D desde la GUI.

## Preparar el escenario

1. Abrí la aplicación en modo preparación.
2. Creá planos o una sala básica.
3. Agregá fuentes y receptores.
4. Ajustá las propiedades relevantes antes de iniciar la simulación.

## Controles rápidos

| Tecla | Acción |
|---|---|
| `P` | Agregar plano. |
| `B` | Agregar sala básica. |
| `C` | Seleccionar el siguiente plano. |
| `J` / `L` | Mover selección en X. |
| `U` / `O` | Mover selección en Y. |
| `I` / `K` | Mover selección en Z. |
| `Tab` | Cambiar entre preparación y simulación. |
| `Enter` | Iniciar la simulación desde el modo simulación. |

## Propiedades importantes

| Panel | Campo | Qué controla |
|---|---|---|
| General | Number of rays | Cantidad global de rayos solicitada para la simulación. El Core puede ajustarla a un conteo icosaédrico válido. |
| Source | Energy | Energía inicial de la fuente. El valor por defecto es `120`. |
| Plane | Absorption | Pérdida de energía aplicada cuando la energía interactúa con esa superficie. |

## Ejecutar y revisar la simulación

1. Cambiá al modo simulación con `Tab`.
2. Iniciá con `Enter`.
3. Usá la pantalla de simulación para observar rayos, energía difusa y energía recibida.
4. Ajustá la velocidad de reproducción si necesitás inspeccionar el resultado con más detalle.

La velocidad visual puede ir de `0.001x` a `2x`. El deslizador usa escala logarítmica para dar más precisión en velocidades pequeñas. Hacé doble click sobre el campo de velocidad si necesitás ingresar un valor exacto. Este control no recalcula el Core; solo cambia cómo se reproduce el resultado ya generado.
