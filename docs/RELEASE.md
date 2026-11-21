# Release history

## v1.1.0: GUI and complete acceptance evidence

This release adds Influence Studio and closes the software gaps identified in the Project 15 audit:

- The loaded-data workbench maps every mandatory family to actual records with explicit bounded analytical projections; N-Queens remains the prescribed validation exercise.
- Powering and celebrity discovery use recursive divide-and-conquer implementations with overflow/boundary and independent oracle tests.
- The benchmark runner records measured peak live C++ heap payload separately for each call. Analytical workspace estimates and cumulative RSS remain distinct.
- Floating solution values, exact references, relative gaps and guarantee labels distinguish exact integer plans, feasible heuristics and fractional bounds.
- `make comparisons` reproduces normal, shock, 10,000-user stress and a controlled adversarial campaign. GUI **Compare this network** uses the active dataset/budget/scenario.
- The full runner has 144 cases across all 36 mandatory variants, including a bounded-adversarial tier.
- User/design/GUI/complexity guides, acceptance checklist, demonstration script, PDF/Word report and PowerPoint include these workflows and limitations.

From a fresh checkout:

```sh
git clone https://github.com/Aymuos22/nikita.git
cd nikita
make -j4
make test
make comparisons
make gui
```

The GUI opens `http://127.0.0.1:8765`. It uses Python 3's standard library and local browser assets, with no pip/npm runtime installation. `run_gui.command` is the macOS launcher. See [GUI.md](GUI.md) for ports, temporary sessions, size limits and Terminal Desktop-folder access.

The current release is tagged `v1.1.0`; `v1.0.0` remains the original C++ CLI baseline. The repository is [Aymuos22/nikita](https://github.com/Aymuos22/nikita). Application hosting is local, not publicly deployed.

## v1.0.0: original CLI baseline

Contains the original 36 algorithm variants, reusable library, CLI, synthetic sample data, benchmark runner, test suite and core documentation. Subsequent GUI and acceptance work does not move this historical tag.

## Evidence boundaries

[REQUIREMENTS.md](REQUIREMENTS.md) maps the brief to implementation and executable evidence. Heap payload is not total resident memory; benchmark allocator bookkeeping affects timings. Campaign optimality is over a bounded additive-score pool. The current core/sanitizer and GUI records are identified separately in `results/`.

Named student ownership, human peer-review sign-off and a recorded presentation remain student-provided evidence in [OWNERSHIP.md](OWNERSHIP.md). No automated release fabricates these records.
