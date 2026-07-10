# ADR 0002: Core as an Internal API

## Status

Accepted

## Context

The project needs a reusable acoustic simulation module for the GUI and tests, but it does not need to publish a standalone SDK or installable external package.

The Core must remain independent from OpenGL, GLFW, GLAD and Dear ImGui so it can be tested and evolved separately from the application window.

## Decision

The Core lives under `src/core`, uses the `core::` namespace and is built as the internal static library `simulation_core`.

The public headers in `src/core` are internal project contracts. They are stable enough for GUI adapters and tests, but they are not treated as an external versioned API.

## Consequences

- `simulation_core` can be linked by the GUI and tests without bringing graphical dependencies.
- Core contracts can evolve with the project when GUI adapters and docs are updated together.
- External installation, package metadata and backwards compatibility guarantees are out of scope.
- Architecture documentation should describe Core usage as internal integration, not as a third-party API.

## References

- `docs/architecture/core/README.md`
- `docs/core-api.md`
- `CMakeLists.txt`
