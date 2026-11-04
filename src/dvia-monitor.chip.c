/*
 * Copyright (C) 2026 B1200 Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wokwi-api.h"

#define MAX_RAW_SAMPLES 512
#define MAX_ACTIVE_W 800
#define MAX_ACTIVE_H 600

typedef uint32_t pixel_t;

/**
 * @struct raw_sample_t
 * @brief Single raw sample of a pixel scan
 *
 * This structure contains a single raw sample of a pixel scan.
 *
 * @var t The timestamp of the sample in nanoseconds
 * The timestamp is in nanoseconds and is relative to the start of the
 * simulation.
 *
 * @var rgba The raw RGB value of the sample (2 bits per channel)
 * The raw RGB value is a 2-bit per channel value, where the least
 * significant bit is the least significant bit of the red channel, the
 * next bit is the least significant bit of the green channel, and the
 * most significant bit is the least significant bit of the blue channel.
 */
typedef struct {
  uint64_t t;   /**< The timestamp of the sample in nanoseconds */
  pixel_t rgba; /**< The raw RGB value of the sample (2 bits per channel) */
} raw_sample_t;

/**
 * @struct dvia_state_t
 * @brief State of the DVI monitor chip
 *
 * This structure contains the state of the DVI monitor chip.
 *
 * @var r0 The R0 pin
 * @var r1 The R1 pin
 * @var g0 The G0 pin
 * @var g1 The G1 pin
 * @var b0 The B0 pin
 * @var b1 The B1 pin
 * @var hsync The HSYNC pin
 * @var vsync The VSYNC pin
 * @var ddcio The DDCIO pin
 * @var ddccl The DDCCL pin
 * @var r_bits The cached R bits
 * @var g_bits The cached G bits
 * @var b_bits The cached B bits
 * @var line_start_time The timestamp of the start of the line
 * @var last_hsync_time The timestamp of the last HSYNC
 * @var hsync_count The number of HSYNC edges encountered
 * @var line_number The number of the line
 * @var active_width The active width of the screen
 * @var active_height The active height of the screen
 * @var next_width The next active width of the screen
 * @var next_height The next active height of the screen
 * @var pixel_period_ns The period of a pixel in nanoseconds
 * @var cfg_width The configured width of the screen
 * @var cfg_height The configured height of the screen
 * @var samples The raw samples collected
 * @var sample_count The number of samples collected
 * @var overflowed Whether the sample buffer has overflowed
 * @var fb The frame buffer
 * @var fb_width The width of the frame buffer
 * @var fb_height The height of the frame buffer
 * @var backbuffer The backbuffer
 * @var last_hsync The last state of the HSYNC pin
 * @var last_vsync The last state of the VSYNC pin
 */
typedef struct {
  /**
   * The R0 pin
   */
  pin_t r0;

  /**
   * The R1 pin
   */
  pin_t r1;

  /**
   * The G0 pin
   */
  pin_t g0;

  /**
   * The G1 pin
   */
  pin_t g1;

  /**
   * The B0 pin
   */
  pin_t b0;

  /**
   * The B1 pin
   */
  pin_t b1;

  /**
   * The HSYNC pin
   */
  pin_t hsync;

  /**
   * The VSYNC pin
   */
  pin_t vsync;

  /**
   * The DDCCL pin
   */
  pin_t ddccl;

  /**
   * The DDCIO pin
   */
  pin_t ddcio;

  /**
   * The cached R bits
   */
  uint8_t r_bits;

  /**
   * The cached G bits
   */
  uint8_t g_bits;

  /**
   * The cached B bits
   */
  uint8_t b_bits;

  /**
   * The timestamp of the start of the line
   */
  uint64_t line_start_time;

  /**
   * The timestamp of the last HSYNC
   */
  uint64_t last_hsync_time;

  /**
   * The number of HSYNC edges encountered
   */
  uint32_t hsync_count;

  /**
   * The number of the line
   */
  uint32_t line_number;

  /**
   * The active width of the screen
   */
  pixel_t active_width;

  /**
   * The active height of the screen
   */
  pixel_t active_height;

  /**
   * The next active width of the screen
   */
  pixel_t next_width;

  /**
   * The next active height of the screen
   */
  pixel_t next_height;

  /**
   * The period of a pixel in nanoseconds
   */
  uint64_t pixel_period_ns;

  /**
   * The configured width of the screen
   */
  pixel_t cfg_width;

  /**
   * The configured height of the screen
   */
  pixel_t cfg_height;

  /**
   * The raw samples collected
   */
  raw_sample_t samples[MAX_RAW_SAMPLES];

  /**
   * The number of samples collected
   */
  uint32_t sample_count;

  /**
   * Whether the sample buffer has overflowed
   */
  bool overflowed;

  /**
   * The frame buffer
   */
  buffer_t fb;

  /**
   * The width of the frame buffer
   */
  pixel_t fb_width;

  /**
   * The height of the frame buffer
   */
  pixel_t fb_height;

  /**
   * The backbuffer
   */
  pixel_t* backbuffer;

  /**
   * The last state of the HSYNC pin
   */
  bool last_hsync;

  /**
   * The last state of the VSYNC pin
   */
  bool last_vsync;
} dvia_state_t;

/**
 * Assert a pin's electrical level based on its active high/low state
 *
 * @param pin The pin to check
 * @param active_high Whether the pin is active high (true) or low (false)
 * @return Whether the pin is currently asserted (true) or deasserted (false)
 */
static inline bool sync_asserted(pin_t pin, bool active_high) {
  bool level = pin_read(pin);
  return active_high ? level : !level;
}

/**
 * Expand a 2-bit color to an 8-bit color by replicating the two bits
 * into the high and low halves.
 *
 * The expansion is done as follows:
 * - 00 -> 0x00
 * - 01 -> 0x55 (01010101b = 85)
 * - 10 -> 0xAA (10101010b = 170)
 * - 11 -> 0xFF (11111111b = 255)
 *
 * @param color_bits 2-bit color to expand
 * @return 8-bit expanded color
 */
static inline uint8_t expand2to8(uint8_t color_bits) {
  color_bits &= 0x03;  // keep only the low 2 bits
  return (color_bits << 6) | (color_bits << 4) | (color_bits << 2) |
         color_bits;  // bit‑replication pattern
}

/**
 * Pack three 2-bit colors into a single 32-bit pixel.
 *
 * @param red 2-bit red color
 * @param green 2-bit green color
 * @param blue 2-bit blue color
 * @return 32-bit packed pixel with A=FF, R, G, B in that order
 */
static inline pixel_t pack_rgba(uint8_t red, uint8_t green, uint8_t blue) {
  uint8_t r = expand2to8(red);
  uint8_t g = expand2to8(green);
  uint8_t b = expand2to8(blue);
  return 0xFF000000 | (b << 16) | (g << 8) | r;
}

/**
 * @brief Handle color pin interrupts
 *
 * This function is called whenever any of the color pins change state.
 * It updates the cached color bits and stores the new color in the sample
 * buffer if it has changed.
 *
 * @param user_data The user-provided data to pass to the interrupt service
 * routine
 * @param pin The pin that triggered the interrupt
 * @param value The new electrical level of the pin
 */
void color_isr(void* user_data, pin_t pin, uint32_t value) {
  dvia_state_t* chip = user_data;

  // Ignore color changes during vertical blanking
  if (chip->last_vsync == HIGH)
    return;

  // Ignore color changes during horizontal blanking
  if (chip->last_hsync == HIGH)
    return;

  // Update the cached color bits
  if (pin == chip->r0)
    chip->r_bits = (chip->r_bits & ~1) | (value & 1);
  else if (pin == chip->r1)
    chip->r_bits = (chip->r_bits & ~2) | ((value & 1) << 1);
  else if (pin == chip->g0)
    chip->g_bits = (chip->g_bits & ~1) | (value & 1);
  else if (pin == chip->g1)
    chip->g_bits = (chip->g_bits & ~2) | ((value & 1) << 1);
  else if (pin == chip->b0)
    chip->b_bits = (chip->b_bits & ~1) | (value & 1);
  else if (pin == chip->b1)
    chip->b_bits = (chip->b_bits & ~2) | ((value & 1) << 1);
  else
    return;

  // Pack the three 2-bit colors into a single 32-bit pixel
  pixel_t rgba = pack_rgba(chip->r_bits, chip->g_bits, chip->b_bits);

  // If the color has changed, store it in the sample buffer
  if (chip->sample_count == 0 ||
      chip->samples[chip->sample_count - 1].rgba != rgba) {
    if (chip->sample_count < MAX_RAW_SAMPLES) {
      chip->samples[chip->sample_count++] =
          (raw_sample_t){get_sim_nanos(), rgba};
    } else if (!chip->overflowed) {
      printf("WARNING: raw RGB overflow on line %u\n", chip->line_number);
      chip->overflowed = true;
    }
  }
}

/**
 * Handle HSYNC pin interrupts
 *
 * This function is called whenever the HSYNC pin changes state. It is
 * responsible for detecting the start and end of each line and
 * rendering the line to the frame buffer.
 *
 * @param user_data The user-provided data to pass to the interrupt service
 * routine
 * @param pin The pin that triggered the interrupt
 * @param value The new electrical level of the pin
 */
void hsync_isr(void* user_data, pin_t pin, uint32_t value) {
  dvia_state_t* chip = user_data;

  // Raw electrical level for edge detection
  bool raw = (bool)value;

  // Rising edge of RAW HSYNC: end of line
  if (!chip->last_hsync && raw) {
    // Compute pixel clock from previous line
    uint64_t now = get_sim_nanos();
    if (chip->last_hsync_time && chip->active_width) {
      uint64_t line_period = now - chip->line_start_time;
      chip->pixel_period_ns = line_period / chip->active_width;

      // Check for invalid pixel clock
      if (chip->pixel_period_ns < 10 || chip->pixel_period_ns > 100000000) {
        printf("WARNING: invalid pixel clock (%lu ns)\n",
               (unsigned long)chip->pixel_period_ns);
        chip->pixel_period_ns = 0;
      }
    }

    // Render line
    if (chip->pixel_period_ns && chip->sample_count && chip->active_width &&
        chip->active_height) {
      uint32_t scale_y = chip->fb_height / chip->active_height;
      if (scale_y == 0)
        scale_y = 1;

      uint32_t src_y = chip->line_number;
      if (src_y < chip->active_height) {
        uint32_t dst_y = (uint64_t)src_y * scale_y;

        uint32_t sample = 0;

        // Iterate over each sample in the line
        for (uint32_t dst_x = 0; dst_x < chip->fb_width; dst_x++) {
          uint32_t src_x =
              (uint64_t)dst_x * chip->active_width / chip->fb_width;

          uint64_t time = chip->line_start_time + src_x * chip->pixel_period_ns;

          // Find the sample that corresponds to the current pixel
          while (sample + 1 < chip->sample_count &&
                 chip->samples[sample + 1].t <= time)
            sample++;

          pixel_t rgba = chip->samples[sample].rgba;

          // Render the sample to the frame buffer
          for (uint32_t i = 0; i < scale_y; i++) {
            pixel_t pixel_index =
                (dst_y * chip->fb_width + dst_x) + i * chip->fb_width;
            chip->backbuffer[pixel_index] = rgba;
          }
        }
      }
    }

    // Reset the sample count and overflow flag
    chip->sample_count = 0;
    chip->overflowed = false;

    // Increment the line number and HSYNC count
    chip->line_number++;
    chip->hsync_count++;

    // Store the current timestamp as the last HSYNC time
    chip->last_hsync_time = now;
  }

  // Falling edge of RAW HSYNC: start of line
  if (chip->last_hsync && !raw) {
    chip->line_start_time = get_sim_nanos();
    chip->sample_count = 0;
    chip->overflowed = false;
  }

  chip->last_hsync = raw;
}

/**
 * Handle VSYNC pin interrupts
 *
 * This function is called whenever the VSYNC pin changes state. It is
 * responsible for detecting the start of each frame, computing the
 * active width and height, copying the backbuffer to the framebuffer,
 * and resetting the HSYNC count and line number.
 *
 * @param user_data The user-provided data to pass to the interrupt service
 * routine
 * @param pin The pin that triggered the interrupt
 * @param value The new electrical level of the pin
 */
void vsync_isr(void* user_data, pin_t pin, uint32_t value) {
  dvia_state_t* chip = user_data;
  // Get the current state of the VSYNC pin
  bool raw = (bool)value;

  // If we are transitioning from low to high, it means a new frame is starting
  if (!chip->last_vsync && raw) {
    // If we have seen any HSYNC pulses, compute the active width and height
    if (chip->hsync_count) {
      chip->next_height = chip->hsync_count;
      chip->next_width =
          (chip->hsync_count * chip->cfg_width + chip->cfg_height / 2) /
          chip->cfg_height;
    }

    // Update the active width and height
    chip->active_width = chip->next_width;
    chip->active_height = chip->next_height;

    // Reset the HSYNC count and line number
    chip->hsync_count = 0;
    chip->line_number = 0;

    // Copy the backbuffer to the framebuffer
    buffer_write(chip->fb, 0, (uint8_t*)chip->backbuffer,
                 chip->fb_width * chip->fb_height * sizeof(pixel_t));

    // Empty the backbuffer
    size_t pixels = chip->fb_width * chip->fb_height;
    memset(chip->backbuffer, 0, pixels * sizeof(pixel_t));
  }

  chip->last_vsync = raw;
}

/**
 * Initialize the DVI monitor chip
 *
 * This function initializes the DVI monitor chip by allocating memory
 * for the chip state, initializing all the pins (color and sync), setting
 * active levels for HSYNC and VSYNC, initializing the framebuffer and
 * backbuffer, and watching the pins.
 *
 * @return void
 */
void chip_init(void) {
  // Allocate memory for dvia_state_t
  dvia_state_t* chip = calloc(1, sizeof(dvia_state_t));

  // Check if allocation was successful
  if (!chip) {
    printf("Failed to allocate memory for dvia_state_t.\n");
    return;
  }

  // Initialize all the pins (color and sync)
  chip->r0 = pin_init("R0", INPUT);
  chip->r1 = pin_init("R1", INPUT);
  chip->g0 = pin_init("G0", INPUT);
  chip->g1 = pin_init("G1", INPUT);
  chip->b0 = pin_init("B0", INPUT);
  chip->b1 = pin_init("B1", INPUT);
  chip->hsync = pin_init("HSYNC", INPUT);
  chip->vsync = pin_init("VSYNC", INPUT);
  // Initialize the configuration width and height
  chip->cfg_width = 4;
  chip->cfg_height = 3;

  // Initialize the framebuffer and backbuffer
  chip->fb = framebuffer_init(&chip->fb_width, &chip->fb_height);

  // Initialize the backbuffer to all black
  chip->backbuffer =
      calloc(1, chip->fb_width * chip->fb_height * sizeof(pixel_t));
  if (!chip->backbuffer) {
    printf("Failed to allocate memory for backbuffer.\n");
    return;
  }

  // Define pin watch configurations for color and sync pins
  const pin_watch_config_t rgb = {
      .edge = BOTH, .pin_change = color_isr, .user_data = chip};
  const pin_watch_config_t hs = {
      .edge = BOTH, .pin_change = hsync_isr, .user_data = chip};
  const pin_watch_config_t vs = {
      .edge = BOTH, .pin_change = vsync_isr, .user_data = chip};

  // Watch the pins
  if (!pin_watch(chip->r0, &rgb) || !pin_watch(chip->r1, &rgb) ||
      !pin_watch(chip->g0, &rgb) || !pin_watch(chip->g1, &rgb) ||
      !pin_watch(chip->b0, &rgb) || !pin_watch(chip->b1, &rgb) ||
      !pin_watch(chip->hsync, &hs) || !pin_watch(chip->vsync, &vs)) {
    printf("Failed to watch pins.\n");
    return;
  }
  printf("[chip-dvia-monitor] initialized\n");
}
