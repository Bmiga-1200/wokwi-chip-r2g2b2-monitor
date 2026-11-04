# Project Context: DVI-A Monitor Wokwi Custom Chip

## Architecture Overview
This repository provides a simulated DVI-A / VGA display monitor for Wokwi. It accepts 6-bit digital color pins (`R0, R1, G0, G1, B0, B1`) and sync lines (`HSYNC, VSYNC`), sampling pixel levels in real-time simulation nanos and outputting to Wokwi's frame buffer display API.

## Tech Stack
- **Language**: C99 targeting `wasm32-unknown-wasi`
- **APIs Used**: Wokwi Chip API (`framebuffer_init`, `buffer_write`, `pin_watch`, `get_sim_nanos`)
- **Resolution**: 640x480 active resolution buffer
- **Test Matrix**: Arduino Mega with `VGAXUA` library (`Pattern`), ESP32-S3 test (`SimplePattern`), Pico test (`PicoVGA`).
- **CI/CD**: GitHub Actions workflow packaging `chip.zip` with release automation.
