# Contributing to Wokwi DVI-A / VGA Monitor Chip

This guide details the internal mechanics of the DVI-A Monitor custom chip, its scanline sampling algorithms, framebuffer pipeline, and guidelines for contributing.

---

## Architecture & Rendering Pipeline

The chip simulates an analog display controller receiving RGB discrete bits and sync pulses:

### 1. Color Bit Expansion & RGBA Packing
* Color inputs are 2 bits per channel ($R_1 R_0, G_1 G_0, B_1 B_0$).
* `expand2to8()` replicates the two bits across 8 bits:
  $$\text{00} \to 0x00,\quad \text{01} \to 0x55,\quad \text{10} \to 0xAA,\quad \text{11} \to 0xFF$$
* `pack_rgba()` packages them into 32-bit ARGB pixels:
  $$\text{Pixel} = 0xFF000000 \mid (B \ll 16) \mid (G \ll 8) \mid R$$

### 2. Scanline Sampling (`color_isr`)
* When any color pin changes, `color_isr` records a `raw_sample_t` with high-resolution simulation nanosecond timestamp (`get_sim_nanos()`) and the packed pixel value.
* Samples are ignored during horizontal blanking (`HSYNC == HIGH`) and vertical blanking (`VSYNC == HIGH`).

### 3. Scanline Rasterization (`hsync_isr`)
* **Falling Edge of HSYNC**: Marks the start of a scanline (`line_start_time = get_sim_nanos()`).
* **Rising Edge of HSYNC**: Marks the end of a scanline:
  * Computes pixel clock:
    $$\text{pixel\_period\_ns} = \frac{\text{now} - \text{line\_start\_time}}{\text{active\_width}}$$
  * Resamples the captured raw transitions across `fb_width` pixels by interpolating timestamp checkpoints.
  * Writes the resulting pixel scanline into `backbuffer`.

### 4. Framebuffer Flip (`vsync_isr`)
* Rising edge of `VSYNC` indicates frame completion:
  * Copies the completed `backbuffer` to Wokwi's simulator display framebuffer via `buffer_write()`.
  * Clears `backbuffer` to black with `memset()`.
  * Resets line counter and HSYNC count for the next frame.

---

## Local Development Setup

### Build using WASI-SDK
```bash
make
```
Target: `dist/dvia-monitor.chip.wasm`.

### Run Automated Tests
```bash
make test
# Test slow-scan sketch
make slow_scan
```

---

## Release Process

1. Commit changes to `main`.
2. Tag with a semver release tag:
   ```bash
   git tag -a v1.0.0 -m "Release v1.0.0"
   git push origin v1.0.0
   ```
3. GitHub Actions builds the WASM binary, runs Arduino Mega tests, and creates a release containing `chip.zip`.
