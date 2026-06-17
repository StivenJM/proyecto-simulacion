# Diseño del Sistema

Este documento define la arquitectura del simulador acústico 3D. El objetivo es que las tres áreas del equipo puedan trabajar de forma independiente: Tests, Core y Animación/GUI.

La decisión principal es que la GUI no dependa directamente del código interno de simulación. La GUI debe consumir servicios definidos por interfaces. Esas interfaces viven en `services`, y sus implementaciones pueden ser mockeadas o reales según la configuración de la aplicación.

## Idea Central

La aplicación se organiza alrededor de una capa `services`.

`services` no significa servicios externos ni servicios web. En este proyecto significa la capa pública que ofrece operaciones a la GUI, como ejecutar una simulación, cargar resultados o consultar datos procesados.

La GUI solo conoce interfaces y factories. No conoce si detrás está funcionando un mock o el Core real.

```text
+-------------------------------------------------------------+
|                            GUI                              |
|     OpenGL, editor visual, camara, vistas y controles        |
+-----------------------------+-------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                          Services                           |
|  Interfaces, tipos compartidos, factory e implementaciones   |
+-----------------------------+-------------------------------+
                              |
             +----------------+----------------+
             |                                 |
             v                                 v
+-----------------------------+   +---------------------------+
|      Mock Implementation     |   |    Core Implementation    |
|   Datos falsos para GUI      |   |  Calculos reales          |
+-----------------------------+   +---------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                            Tests                            |
|      Validan Core usando interfaces y escenarios fixture     |
+-------------------------------------------------------------+
```

## Objetivos de Arquitectura

- Permitir que Core, GUI y Tests avancen en paralelo.
- Permitir que GUI funcione con servicios mock mientras Core se completa.
- Permitir que GUI cambie de mock a Core real sin cambiar su flujo principal.
- Permitir que Tests validen Core sin depender de OpenGL.
- Mantener configuración global en un lugar claro.
- Evitar que Core dependa de ventanas, renderizado, teclado, mouse o shaders.
- Evitar que GUI llame funciones internas del Core.

## División del Equipo

| Área | Responsabilidad | No debe encargarse de |
|---|---|---|
| Tests | Tests unitarios e integración del Core | Implementar GUI o depender de OpenGL |
| Core | Cálculos de simulación acústica, ray tracing, energía difusa, tiempos, porcentajes y resultados | Renderizado, cámara, ventanas, controles visuales |
| Animación/GUI | Visualización 3D, edición del escenario, modos, cámara y presentación de resultados | Fórmulas físicas o validación numérica del Core |

## Estructura Recomendada

```text
src/
  config/
    AppConfig.h
    AppConfig.cpp
    ServiceMode.h

  services/
    SimulationTypes.h
    ISimulationService.h
    ServiceFactory.h
    ServiceFactory.cpp

    implementations/
      mock/
        MockSimulationService.h
        MockSimulationService.cpp

      core/
        CoreSimulationService.h
        CoreSimulationService.cpp
        GeometryCalculator.h
        GeometryCalculator.cpp
        RayTracer.h
        RayTracer.cpp
        DiffuseEnergySolver.h
        DiffuseEnergySolver.cpp

  gui/
    App.h
    App.cpp
    SceneEditor.h
    SceneEditor.cpp
    SimulationView.h
    SimulationView.cpp
    OpenGLRenderer.h
    OpenGLRenderer.cpp
    CameraController.h
    CameraController.cpp

  io/
    ScenarioFile.h
    ScenarioFile.cpp
    ResultExporter.h
    ResultExporter.cpp

tests/
  services/
  core/
  fixtures/

workspace/
  core-initial/
```

## Capa Config

`config` guarda parámetros globales de la aplicación. Su responsabilidad no es calcular ni renderizar, sino decidir cómo se comporta la aplicación a nivel general.

Ejemplos de configuración:

- Usar servicio mock o servicio Core real.
- Definir duración por defecto de simulación.
- Definir velocidad del sonido por defecto.
- Definir cantidad de rayos por defecto.
- Activar o desactivar datos de depuración.

### ServiceMode

```cpp
enum class ServiceMode {
    Mock,
    Core
};
```

### AppConfig

```cpp
struct AppConfig {
    ServiceMode serviceMode = ServiceMode::Mock;
    int defaultDurationMs = 1000;
    double defaultSoundSpeed = 340.0;
    int defaultRayCount = 642;
    bool debugEnabled = false;
};
```

Regla:

- La GUI puede leer configuración de aplicación.
- Los servicios pueden recibir configuración.
- El Core no debe leer configuración global directamente si eso dificulta los tests. Para cálculos, debe recibir parámetros explícitos.

## Capa Services

`services` contiene lo que la GUI puede usar. Dentro de esta carpeta viven tanto los contratos como las implementaciones.

La estructura recomendada es:

```text
services/
  SimulationTypes.h
  ISimulationService.h
  ServiceFactory.h
  ServiceFactory.cpp
  implementations/
    mock/
    core/
```

## Tipos Compartidos del Servicio

Estos tipos son el idioma común entre GUI, Core, Mock, IO y Tests.

### Datos Base

```cpp
struct Vec3 {
    double x;
    double y;
    double z;
};

struct TriangleData {
    int id;
    int surfaceId;
    Vec3 a;
    Vec3 b;
    Vec3 c;
};

struct SurfaceData {
    int id;
    double absorption;
    std::vector<TriangleData> triangles;
};

struct SourceData {
    int id;
    Vec3 position;
    double energy;
};

struct ReceiverData {
    int id;
    Vec3 position;
    double radius;
};
```

### Escenario

```cpp
struct ScenarioData {
    std::vector<SurfaceData> surfaces;
    std::vector<SourceData> sources;
    std::vector<ReceiverData> receivers;
};
```

La GUI construye `ScenarioData` desde el editor visual. El servicio de simulación recibe ese escenario.

### Configuración de Simulación

```cpp
struct SimulationConfig {
    int durationMs = 1000;
    double soundSpeed = 340.0;
    int rayCount = 642;
    double diffusionCoefficient = 0.5;
};
```

La absorción se define por superficie en `SurfaceData`, porque el proyecto necesita coeficientes distintos por plano.

### Resultado de Simulación

```cpp
struct ReflectionRaySegment {
    Vec3 from;
    Vec3 to;
    double energy;
    int timeMs;
};

struct ReceiverEnergySample {
    int receiverId;
    int timeMs;
    double energy;
};

struct TriangleEnergySample {
    int triangleId;
    int timeMs;
    double energy;
};

struct DiffusionMatrixData {
    std::vector<std::vector<double>> distances;
    std::vector<std::vector<int>> timesMs;
    std::vector<std::vector<double>> percentages;
    std::vector<std::vector<bool>> visibility;
};

struct SimulationResult {
    bool success;
    std::string message;
    std::vector<ReflectionRaySegment> reflectionRays;
    std::vector<ReceiverEnergySample> receiverEnergy;
    std::vector<TriangleEnergySample> triangleEnergy;
    DiffusionMatrixData diffusion;
    double totalReceiverEnergy;
    double lostEnergy;
};
```

Estos datos permiten que GUI dibuje rayos, coloree superficies, muestre energía recibida y exporte resultados.

## Interfaz del Servicio de Simulación

La interfaz es el contrato que consume GUI.

Archivo sugerido:

```text
src/services/ISimulationService.h
```

```cpp
class ISimulationService {
public:
    virtual ~ISimulationService() = default;

    virtual SimulationResult runSimulation(
        const ScenarioData& scenario,
        const SimulationConfig& config
    ) = 0;
};
```

Regla:

- GUI solo usa `ISimulationService`.
- GUI no usa `CoreSimulationService` directamente.
- GUI no usa funciones internas como `calcularMatricesVisibilidad` o `propagarEnergiaDifusa`.

## Implementación Mock

Archivo sugerido:

```text
src/services/implementations/mock/MockSimulationService.h
src/services/implementations/mock/MockSimulationService.cpp
```

```cpp
class MockSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData& scenario,
        const SimulationConfig& config
    ) override;
};
```

El mock debe devolver datos falsos pero coherentes:

- Rayos visibles para probar la visualización.
- Energía por receptor para probar paneles de resultados.
- Energía por triángulo para probar mapas de color.
- Matrices pequeñas para probar tablas o exportación.

Esto desbloquea a la persona de Animación/GUI. Puede construir la interfaz completa sin esperar a que Core termine.

## Implementación Core

Archivo sugerido:

```text
src/services/implementations/core/CoreSimulationService.h
src/services/implementations/core/CoreSimulationService.cpp
```

```cpp
class CoreSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData& scenario,
        const SimulationConfig& config
    ) override;
};
```

`CoreSimulationService` es la implementación real. Internamente puede usar clases auxiliares como:

- `GeometryCalculator`
- `RayTracer`
- `DiffuseEnergySolver`

Pero esas clases no deben ser usadas por GUI.

## Factory de Servicios

El factory decide qué implementación entregar según la configuración.

Archivo sugerido:

```text
src/services/ServiceFactory.h
src/services/ServiceFactory.cpp
```

```cpp
class ServiceFactory {
public:
    static std::unique_ptr<ISimulationService> createSimulationService(
        const AppConfig& config
    );
};
```

Implementación conceptual:

```cpp
std::unique_ptr<ISimulationService> ServiceFactory::createSimulationService(
    const AppConfig& config
) {
    if (config.serviceMode == ServiceMode::Mock) {
        return std::make_unique<MockSimulationService>();
    }

    return std::make_unique<CoreSimulationService>();
}
```

La GUI usa el factory:

```cpp
AppConfig config = loadAppConfig();
auto simulationService = ServiceFactory::createSimulationService(config);

App app(std::move(simulationService));
app.run();
```

Así la GUI no decide manualmente qué implementación usar. Esa decisión queda centralizada en `config` + `ServiceFactory`.

## Flujo de Preparación

```text
Usuario edita escenario en GUI
        |
GUI construye ScenarioData
        |
Usuario coloca fuentes y receptores
        |
Usuario configura absorción por superficie
        |
Usuario guarda o comparte el escenario si lo desea
```

## Flujo de Simulación

```text
Usuario presiona iniciar simulación
        |
GUI crea SimulationConfig
        |
GUI llama ISimulationService::runSimulation
        |
Factory ya decidió si el servicio es Mock o Core
        |
Servicio devuelve SimulationResult
        |
GUI visualiza rayos, energía y resultados
        |
Usuario exporta datos y visualizaciones si lo desea
```

## Relación Con el Código Inicial de Core

El código de `workspace/core-initial` contiene lógica útil, pero actualmente está acoplado a un programa de consola.

Conceptos que se deben preservar:

- Punto, vector y triángulo.
- Cálculo de centroides.
- Cálculo de áreas.
- Cálculo de distancias entre centroides.
- Cálculo de tiempos de vuelo con velocidad del sonido.
- Matrices de distancia, tiempo, porcentaje y visibilidad.
- Ray tracing para impactos de rayos.
- Separación de energía especular y difusa.
- Energía recibida por receptores.

Cambios necesarios para integrarlo:

- Mover la lógica real a `CoreSimulationService` y clases auxiliares.
- Eliminar dependencia de menús de consola.
- Evitar `cin` y `cout` como parte del cálculo.
- Reemplazar variables globales por estado interno controlado.
- Devolver `SimulationResult` en memoria.
- Separar guardado de archivos hacia `io/ResultExporter`.
- Soportar absorción por superficie.
- Soportar los datos que lleguen desde `ScenarioData`.

## Tests Independientes

Los Tests deben validar la implementación Core sin depender de OpenGL.

### Regla Principal

Tests no prueba GUI. Tests prueba `CoreSimulationService` y las reglas de simulación.

### Tests Unitarios

| Test | Valida |
|---|---|
| Centroide | Centro de un triángulo conocido |
| Distancia | Distancia entre dos puntos conocidos |
| Tiempo de vuelo | 340 m equivalen a 1000 ms con velocidad 340 m/s |
| Visibilidad | Triángulos del mismo plano no son visibles entre sí |
| Porcentajes | Los porcentajes de destinos visibles suman aproximadamente 1 |
| Absorción | La energía disminuye según el coeficiente de la superficie |

### Tests de Integración del Core

Los tests de integración usan escenarios pequeños completos.

Ejemplos:

- Escenario con dos triángulos visibles.
- Escenario con dos triángulos en el mismo plano.
- Sala simple con fuente y receptor.
- Escenario con absorciones distintas por superficie.

### Fixtures

Ubicación sugerida:

```text
tests/fixtures/ScenarioFixtures.h
```

Ejemplo:

```cpp
ScenarioData makeTwoVisibleTrianglesScenario();
ScenarioData makeSamePlaneTrianglesScenario();
ScenarioData makeSimpleRoomScenario();
```

### Test Contra la Interfaz

```cpp
void runBasicSimulationTest(ISimulationService& service) {
    ScenarioData scenario = makeSimpleRoomScenario();
    SimulationConfig config;
    config.durationMs = 1000;

    SimulationResult result = service.runSimulation(scenario, config);

    // Validar éxito, energía, tiempos y matrices esperadas.
}
```

Esto permite probar `CoreSimulationService` sin que el test conozca detalles internos del Core.

## Persistencia y Exportación

La persistencia y exportación no deben vivir dentro del cálculo.

Ubicación sugerida:

```text
src/io/
  ScenarioFile.h
  ScenarioFile.cpp
  ResultExporter.h
  ResultExporter.cpp
```

Responsabilidades:

- Guardar y cargar `ScenarioData` para compartir escenarios.
- Exportar datos de `SimulationResult`.
- Exportar capturas o información visual preparada por GUI.

El servicio de simulación no debe depender de estos exportadores para funcionar.

## Dependencias Permitidas

```text
gui                     -> services/interfaces + ServiceFactory
ServiceFactory          -> config + mock implementation + core implementation
mock implementation     -> services/interfaces
core implementation     -> services/interfaces
io                      -> services/types
tests                   -> services/interfaces + core implementation
config                  -> sin dependencias de GUI/Core
```

Dependencias prohibidas:

```text
core implementation -> gui
core implementation -> OpenGL
core implementation -> tests
tests               -> gui/OpenGL
gui                 -> funciones internas del core
gui                 -> workspace/core-initial
```

## Targets de Compilación Recomendados

| Target | Tipo | Contiene | Depende de |
|---|---|---|---|
| `app_config` | Librería | Configuración global | Nada del proyecto |
| `simulation_services` | Librería | Tipos, interfaces y factory | `app_config` |
| `simulation_service_mock` | Librería | Implementación mock | `simulation_services` |
| `simulation_service_core` | Librería | Implementación real del Core | `simulation_services` |
| `simulation_gui` | Librería o parte del ejecutable | OpenGL, editor, vistas y cámara | `simulation_services` |
| `simulation_io` | Librería | Carga/guardado y exportación | `simulation_services` |
| `AcousticSimulator` | Ejecutable | Composición final | GUI, services, config, mock/core, io |
| `core_tests` | Ejecutable de tests | Tests del Core | `simulation_services`, `simulation_service_core` |

Reglas importantes:

- `simulation_service_core` debe compilar sin GLFW, GLAD ni OpenGL.
- `core_tests` debe ejecutarse sin abrir ventanas.
- `simulation_gui` debe poder funcionar con `simulation_service_mock` aunque `simulation_service_core` todavía no esté terminado.

## Plan de Integración

### Fase 1: Config y Services Base

Crear:

- `AppConfig`
- `ServiceMode`
- `SimulationTypes`
- `ISimulationService`
- `ServiceFactory`

Resultado esperado:

- GUI sabe qué servicio consumir.
- Core sabe qué interfaz implementar.
- Tests saben qué tipos usar.

### Fase 2: MockSimulationService

Crear una implementación mock con datos falsos pero coherentes.

Resultado esperado:

- GUI puede mostrar simulaciones falsas.
- Animación/GUI no se bloquea por el avance del Core.

### Fase 3: CoreSimulationService

Adaptar la lógica útil de `workspace/core-initial` a la implementación real.

Resultado esperado:

- La simulación real responde mediante `ISimulationService`.
- No hay dependencia de consola dentro del servicio.
- Los resultados se entregan como `SimulationResult`.

### Fase 4: Tests del Core

Crear tests unitarios y de integración sobre `CoreSimulationService`.

Resultado esperado:

- Se validan cálculos críticos sin OpenGL.
- Los cambios del Core se pueden verificar rápido.

### Fase 5: Integración GUI + Core

Cambiar `AppConfig.serviceMode` de `Mock` a `Core`.

Resultado esperado:

- La GUI empieza a mostrar resultados reales sin cambiar su código principal.

## Decisiones Arquitectónicas

| Decisión | Motivo |
|---|---|
| Usar `services` como capa pública | GUI consume servicios, no código interno del Core |
| Poner interfaces dentro de `services` | El contrato vive junto al servicio que representa |
| Separar implementaciones en `services/implementations` | Mock y Core quedan claros y reemplazables |
| Usar `config` para elegir mock o Core | La decisión no queda repartida por la GUI |
| Usar `ServiceFactory` | Centraliza la creación del servicio correcto |
| Tests contra `CoreSimulationService` | Valida Core sin depender de GUI ni OpenGL |
| Exportación en `io` | Evita mezclar cálculo con formatos de salida |

## Definición de Listo Para Integración

La GUI está lista cuando:

- Construye `ScenarioData` desde el editor visual.
- Usa `ISimulationService` para ejecutar simulaciones.
- Obtiene el servicio mediante `ServiceFactory`.
- Funciona con `MockSimulationService`.

El Core está listo cuando:

- Implementa `CoreSimulationService`.
- Recibe `ScenarioData` y `SimulationConfig`.
- Devuelve `SimulationResult`.
- No depende de OpenGL ni consola.
- Pasa tests unitarios principales.

Tests está listo cuando:

- Tiene fixtures de escenarios simples.
- Valida reglas críticas del Core.
- Ejecuta sin abrir ventanas.
- Puede probar `CoreSimulationService` directamente.
