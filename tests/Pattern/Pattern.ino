#include <VGAXUA.h>

VGAXUA vga;

/**
 * @brief Initialization function for the test.
 *
 * Initializes the VGAXUA library and clears the screen with a black
 * background. Then, it draws a white border around the screen and fills
 * the rest of the screen with random pixels.
 */
void setup() {
  vga.begin();
  vga.clear(91);
  for (int y=0; y!=VGAX_HEIGHT; y++) {
    for (int x=0; x!=VGAX_WIDTH; x++) {
      if (x==0 || x==VGAX_WIDTH-1 || y==0 || y==VGAX_HEIGHT-1)
        vga.putpixel(x, y, 1);
      else
        vga.putpixel(x, y, vga.rand()%2);
    }
  }
}
/**
 * @brief Loop function for the test.
 *
 * Sets the extended colors mask to 0b11 in the loop function.
 */
void loop() {
  vga.setExtendedColorsMask(0b11);
}