# Release history and current GUI update

## Current main: Influence Studio GUI

The GUI update adds a local browser dashboard backed by the existing C++17 engine. It includes an interactive directed graph and score spotlight, complete searchable rankings, community browsing, budget-constrained campaign planning, creator-topic assignments, CSV import/generation, normal/shock scenarios, report downloads, routing/text strategy controls, the complete algorithm workbench and benchmark downloads.

Start from a fresh checkout:

```sh
git clone https://github.com/Aymuos22/nikita.git
cd nikita
make -j4
make test
make gui
```

The browser opens `http://127.0.0.1:8765`. Keep the server terminal open; Ctrl+C stops it. The C++ library and CLI remain independently usable. The GUI adds Python 3 standard-library and browser requirements; it does not need pip/npm packages at runtime. `run_gui.command` is a macOS double-click launcher. See [GUI.md](GUI.md) for ports, session limits and macOS Desktop-folder access.

GUI verification adds seven grouped HTTP integration tests against the real engine and 61 automated Chromium workflow/responsive-layout checks. The original 15,163 unit/oracle and 46 CLI integration checks still pass. The tracked [GUI verification record](../results/GUI_VALIDATION.txt) distinguishes current checks from the original core sanitizer audit.

The report, PowerPoint, Word document, live run sheet, screenshots, architecture, user guide and demonstration instructions now cover the GUI. Documentation is generated from `scripts/build_presentation.py`; its optional document-building dependencies are separate from application requirements. The GUI update is on `main`; it does not move the historical `v1.0.0` tag.

## v1.0.0: original C++ CLI baseline

The original release contains all 36 mandatory algorithm variants, the network analytics CLI, reusable static library, sample data, reproducible benchmark runner, tests and core documentation. It retains the candidate, instance-size and measurement limits described in the README. The annotated `v1.0.0` tag identifies that source state; later GUI and presentation work is on `main`.

The repository remote is [Aymuos22/nikita](https://github.com/Aymuos22/nikita). No public application hosting or platform integration is configured: the GUI runs on the user's workstation. Named student ownership, human peer review and a recorded presentation remain student-provided evidence, tracked in [OWNERSHIP.md](OWNERSHIP.md).
