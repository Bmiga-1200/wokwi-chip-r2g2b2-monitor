#include <Arduino.h>

// Pin mapping – adjust to match your Wokwi wiring
const int red0 = 3;
const int red1 = 2;
const int green0 = 15;
const int green1 = 16;
const int blue0 = 17;
const int blue1 = 18;
const int HSYNC = 9;
const int VSYNC = 11;

void setColor(uint8_t r = 0,
              uint8_t g = 0,
              uint8_t b = 0,
              uint32_t delay_time = 50) {
  digitalWrite(red0, r & 0x01);
  digitalWrite(red1, (r >> 1) & 0x01);

  digitalWrite(green0, g & 0x01);
  digitalWrite(green1, (g >> 1) & 0x01);

  digitalWrite(blue0, b & 0x01);
  digitalWrite(blue1, (b >> 1) & 0x01);

  delay(delay_time);
}

void setup() {
  pinMode(red0, OUTPUT);
  pinMode(red1, OUTPUT);
  pinMode(green0, OUTPUT);
  pinMode(green1, OUTPUT);
  pinMode(blue0, OUTPUT);
  pinMode(blue1, OUTPUT);
  pinMode(HSYNC, OUTPUT);
  pinMode(VSYNC, OUTPUT);

  // Idle state
  digitalWrite(HSYNC, HIGH);  // blanking
  digitalWrite(VSYNC, HIGH);  // not in frame
}

void loop() {
  static bool blank = false;
  static uint32_t delay_time = 1;
  // ---- Start of frame ----
  digitalWrite(VSYNC, LOW);  // VSYNC inactive
  // delay(2 * delay_time);

  for (int line = 0; line < 12; line++) {
    // ---- Start of line ----
    digitalWrite(HSYNC, LOW);  // active video
    // delay(delay_time);

    if (!blank) {
      // Emit a few very slow “pixels”
      setColor(0, 0, 0, 0);                    // black
      setColor(0b01, 0, 0, delay_time);        // red-ish
      setColor(0b11, 0, 0, delay_time);        // red
      setColor(0, 0b01, 0, delay_time);        // green-ish
      setColor(0, 0b11, 0, delay_time);        // green
      setColor(0, 0, 0b01, delay_time);        // blue-ish
      setColor(0, 0, 0b11, delay_time);        // blue
      setColor(0b11, 0b11, 0b11, delay_time);  // white (all bits)
      setColor(0, 0, 0, 0);                    // black
    } else {
      // Emit a few very slow “pixels”
      setColor(0, 0, 0, 0);                    // black
      setColor(0, 0b01, 0, delay_time);        // green-ish
      setColor(0b01, 0, 0, delay_time);        // red-ish
      setColor(0, 0, 0b01, delay_time);        // blue-ish
      setColor(0b11, 0b11, 0b11, delay_time);  // white (all bits)
      setColor(0, 0, 0b11, delay_time);        // blue
      setColor(0b11, 0, 0, delay_time);        // red
      setColor(0, 0b11, 0, delay_time);        // green
      setColor(0, 0, 0, 0);                    // black
    }

    // ---- End of line ----
    digitalWrite(HSYNC, HIGH);
    delay(2 * delay_time);
  }
  blank = !blank;

  // ---- End of frame ----
  digitalWrite(VSYNC, HIGH);
  delay(100);
}
