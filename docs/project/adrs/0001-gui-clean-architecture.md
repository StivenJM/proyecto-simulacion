# ADR 0001: GUI Clean Architecture

## Status

Accepted

## Context

The GUI must advance independently while the acoustic Core is adapted from the existing console minicore in `workspace/core-initial` (this ia a file containing basic,no-validated code that is not uploaded to repository).

The GUI needs its own visual domain, screens, rendering, input, and service boundaries without depending on Core internals such as `room`, `plane`, `triangle`, `source`, or `receptor`.

## Decision

The GUI will use its own Clean Architecture under `src/gui`.

GUI-specific `services`, `factories`, `config`, and `composition` will live inside `src/gui`, not at the project root.

All GUI layers will use `src/gui/entities` as the GUI domain model. Data entering or leaving the GUI must pass through DTOs in `src/gui/services/dtos`.

## Structure

```text
src/gui/
  entities/
  services/
    dtos/
    implementations/
      mock/
      core/
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
- Future Core implementations must adapt Core data into GUI DTOs/entities.
- Core must remain independent from GUI, OpenGL, GLFW, and GLAD.

## References

- `docs/architecture/system-design.md`
- `docs/architecture/gui/README.md`
