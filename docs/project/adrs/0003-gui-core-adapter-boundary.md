# ADR 0003: GUI-Core Adapter Boundary

## Status

Accepted

## Context

The GUI has its own visual domain (`GuiScenario`, `GuiPlane`, `GuiSource`, `GuiReceiver`) while the Core exposes calculation-oriented data (`core::ScenarioData`, `core::SourceData`, `core::SimulationConfig`, `core::SimulationResult`).

Letting screens or GUI service interfaces expose Core types would couple UI workflows to simulation internals and make future Core changes harder to contain.

## Decision

Core integration is isolated under `src/gui/services/implementations/core`.

Adapters call `simulation_core`, and mappers under `src/gui/services/implementations/core/mappers` translate between GUI entities/DTOs and Core data.

GUI screens, GUI entities and GUI service interfaces must remain expressed in GUI types.

## Consequences

- Core can change internal structures without forcing screen-level rewrites.
- The GUI can switch between mock and Core implementations through factories/composition.
- Mapping logic has an explicit home and should not be duplicated in screens.
- Reviewers can audit integration by checking the adapter and mapper layer first.

## References

- `docs/architecture/system-design.md`
- `docs/architecture/gui/README.md`
- `src/gui/services/implementations/core`
