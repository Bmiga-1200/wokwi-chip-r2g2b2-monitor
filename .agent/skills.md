# Skills & Operational Commands: DVI-A Monitor Chip

## Local Compilation
```bash
make
```

## Running Tests
```bash
# Pattern test with VGAXUA library
make test

# Slow scan test
make slow_scan
```

## Creating a GitHub Release
```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```
Triggered GitHub Actions publishes `chip.zip` with `chip.json`, `chip.wasm`, `dvia-monitor.chip.json`, and `dvia-monitor.chip.wasm`.
