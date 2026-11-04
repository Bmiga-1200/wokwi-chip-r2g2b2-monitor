# Active Task: DVI-A / VGA Monitor Wokwi Custom Chip

## Current Goal
Maintain and extend the DVI-A (R2G2B2 64-color) simulated monitor custom chip for Wokwi, including scanline edge sampling, dynamic pixel clock calculation, frame buffer rendering, CI/CD testing, and GitHub release automation.

## Status & Progress
- Source code in `src/dvia-monitor.chip.c`.
- Pin definition in `dvia-monitor.chip.json` and `chip.json`.
- GitHub Actions workflow configured in `.github/workflows/build.yaml`.
- Automated test suites in `tests/Pattern/`, `tests/slow_scan/`, `tests/PicoVGA/`, `tests/SimplePattern/`.
- Documentation (`README.md`, `CONTRIBUTING.md`, wiring placeholder) complete.

## Next Steps
