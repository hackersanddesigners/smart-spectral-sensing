#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 1500

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

int width = 160;

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK);
  
}

void loop() {
  tft.fillScreen(TFT_YELLOW);
  tft.drawString("H&D", (width/2)-15, 20, 2);    // string, x, y, size
  tft.drawString("", 0, 60, 7);

  delay(WAIT);
  tft.fillScreen(TFT_YELLOW);

  delay(WAIT);

}
