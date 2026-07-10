# ADR 0004: Global Ray Count with Icosahedral Direction Generation

## Status

Accepted

## Context

The simulation needs a user-configurable ray count while also requiring well-distributed directions over a sphere. Arbitrary counts can produce uneven sampling if directions are generated naively.

The GUI needs a simple global control, not per-source ray settings, so users can tune simulation density from the scenario's general properties.

## Decision

The GUI stores ray count as global scenario configuration in `GuiScenario::simulationConfig.rayCount` and passes it to `core::SimulationConfig::rayCount` when running the Core.

The Core generates directions by subdividing an icosahedron and adjusts the requested count to the valid form:

```text
2 + 10 * n^2
```

## Consequences

- Users configure one global ray count for the prepared scenario.
- The Core preserves regular spherical distribution even when the requested number is not exactly representable.
- Documentation and UI should describe the value as requested/configured ray count, not a guaranteed exact number of generated directions.
- Source energy remains per-source; ray count remains global.

## References

- `docs/architecture/core/README.md`
- `docs/architecture/gui/README.md`
- `src/core/RayTracer.h`
- `src/gui/entities/GuiSimulationConfig.h`
