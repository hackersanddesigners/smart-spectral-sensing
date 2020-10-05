
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "KarlaReg20.h"           // == name of the .h file (tab)

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define KARLA_20 KarlaReg20           // == name given inside .h tab (after "const uint8_t")

void setup() {
  tft.init();
  tft.setRotation(1);
}

void loop() {

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(6);
  tft.setCursor (0, 0, 1);   // set x,y, and font nr
  tft.println("font 1");
  delay(500);
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(6);
  tft.setCursor (0, 0, 2);   // set x,y, and font nr
  tft.println("font 2");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0, 4);   // set x,y, and font nr
  tft.println("font 4");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0, 4);   // set x,y, and font nr
  tft.println("font 4");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0, 4);   // set x,y, and font nr
  tft.println("font 4");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);


  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);
  

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);


  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);

  
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);


  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(6);
  tft.setCursor (0, 0, 6);   // set x,y, and font nr
  tft.println("font 6");
  delay(500);


  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(6);
  tft.setCursor (0, 0, 7);   // set x,y, and font nr
  tft.println("font 7");
  delay(500);

// LOADING A CUSTOM FONT

  tft.loadFont(KARLA_20); // Load font you want to use
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);

    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);
  
    tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);

      tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);

      tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);

      tft.fillScreen(TFT_BLACK);
  tft.setTextSize(6);
  tft.setCursor (0, 0);   // set x,y
  tft.println("custom");
  delay(500);
  tft.unloadFont(); // Remove the font to recover memory used

}
