#include "ESP32S3VGA.h"

// //                   r,r,r,r,r,  g,g, g, g, g, g,   b, b, b, b, b,   h,v
// const PinConfig pins(4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
// 21, 1, 2);
const uint8_t r0 = 11;
const uint8_t r1 = 10;
const uint8_t g0 = 40;
const uint8_t g1 = 45;
const uint8_t b0 = 39;
const uint8_t b1 = 46;
const uint8_t hs = 47;
const uint8_t vs = 48;

const uint8_t led_red = 4;
const uint8_t led_green = 5;

const PinConfig pins(-1,   // r
                     -1,   // r
                     -1,   // r
                     r0,   // r
                     r1,   // r
                     -1,   // g
                     -1,   // g
                     -1,   // g
                     -1,   // g
                     g0,   // g
                     g1,   // g
                     -1,   // b
                     -1,   // b
                     -1,   // b
                     b0,   // b
                     b1,   // b
                     hs,   // hsync
                     vs);  // vsync

VGA vga;

void setup() {
  Serial.begin(115200);  // Start serial communication
  while (!Serial) {
    delay(10);  // Wait for serial monitor to connect
  }
  delay(1000);  // Give time for the monitor to connect
  Serial.println("Starting VGA...");

  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  bool value = 0;

  Mode mode = Mode::MODE_320x240x60;
  bool is_vga_enabled = vga.init(pins, mode, 8);
  if (!is_vga_enabled) {
    Serial.println("VGA init failed");
    while (1) {
      delay(100);
      digitalWrite(led_red, value);
      value = !value;
    }
    return;
  }

  for (int y = 0; y < mode.vRes; y++)
    for (int x = 0; x < mode.hRes; x++)
      vga.dotdit(x, y, x, y, 255 - x);
  for (int y = 0; y < 30; y++)
    for (int x = 0; x < 256; x++) {
      vga.dot(x, y, x, 0, 0);
      vga.dot(x, y + 30, 0, x, 0);
      vga.dot(x, y + 60, 0, 0, x);
    }
  vga.show();
  vga.start();
}

void loop() {
  bool value = 0;
  while (1) {
    digitalWrite(led_green, value);
    value = !value;
    delay(500);
  }
}
