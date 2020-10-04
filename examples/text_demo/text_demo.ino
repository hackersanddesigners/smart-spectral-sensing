#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 1500

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

int width = 160;

void setup() {
  tft.init();
  tft.setRotation(1);
}

void loop() {
  tft.setTextSize(1);
  tft.fillScreen(TFT_YELLOW);
  tft.setTextColor(TFT_BLACK, TFT_CYAN);

  tft.drawString("HACKERS", 15, 20, 2);    // string, x, y, size
  tft.drawString("&", (width/2)-5, 20, 2);    // string, x, y, size

  delay(WAIT);

  tft.fillScreen(TFT_YELLOW);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  tft.drawString("DESIGNERS", (width/2)+5, 20, 2); // string, x, y, size
  tft.drawString("&", (width/2)-5, 20, 2);    // string, x, y, size

  delay(WAIT);

}
