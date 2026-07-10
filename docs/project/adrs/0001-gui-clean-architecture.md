# ADR 0001: GUI Clean Architecture

## Status

Accepted

## Context

The GUI must support scenario editing, simulation visualization and Core integration without coupling screens to acoustic calculation internals.

The GUI needs its own visual domain, screens, rendering, input, and service boundaries. Core integration is allowed through adapters, but GUI public contracts must not expose `core::` types.

## Decision

The GUI will use its own Clean Architecture under `src/gui`.

GUI-specific `services`, `factories`, `config`, and `composition` will live inside `src/gui`, not at the project root.

All GUI layers will use `src/gui/entities` as the GUI domain model. Data entering or leaving GUI service boundaries must pass through GUI DTOs and, when the Core is involved, through mappers under `src/gui/services/implementations/core/mappers`.

## Structure

```text
src/gui/
  entities/
  services/
    dtos/
    implementations/
      mock/
      core/
        mappers/
  factories/
  config/
  composition/
  screens/
  rendering/
  input/
```

## Consequences

- GUI screens depend on GUI entities and service interfaces, not Core classes.
- Rendering maps GUI entities to OpenGL data; it does not own scenario rules.
- Mock implementations can support GUI development before Core integration is complete.
- Core implementations adapt Core data into GUI DTOs/entities through mappers.
- Core must remain independent from GUI, OpenGL, GLFW, and GLAD.
- Public GUI contracts must stay stable even if `core::` types change.

## References

- `docs/architecture/system-design.md`
- `docs/architecture/gui/README.md`
