
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "KarlaReg20.h"           // == name of the .h file (tab)

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

// Maximum number of generations until the screen is refreshed
#define MAX_GEN_COUNT 500

// Define the fonts you want to use (see User_Custom_Fonts.h in TFT lib)
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define KARLA_20 KarlaReg20           // == name given inside .h tab (after "const uint8_t")

int x = 0;
int y = 25;
bool orientation = 0;     // 0 = portrait, 1 = landscape
int width = 70;
int length = 150;
int maxSize = 0;
int xDirection = 5;
int margin = 0;

void setup() {
  
  //Set up the display
  tft.init();
  tft.setRotation(orientation);     
}

void loop() {

  tft.loadFont(KARLA_20); // Load font you want to use
  
  if (orientation == 0){
    maxSize = width-margin;
    }else{
      maxSize = length-margin;
      }
      
  
  tft.fillScreen(TFT_VIOLET);        //set background color, specified in TFT_eSPI.h file
  tft.setTextSize(1);                //text size
  tft.setTextColor(TFT_GREENYELLOW);
//  tft.setCursor(x, y);
  tft.drawCircle(x, y, 10, TFT_GREENYELLOW),
//  tft.println(x);

   tft.setTextSize(3);                //text size
   tft.setCursor(20, 75);      // x,y where the word is drawn
   tft.println(F("H&D"));      

  if (x > maxSize){
    xDirection *= -1;
    }else if (x < 0){
    xDirection *= -1;
      }

    x = x + xDirection;

  tft.unloadFont(); // Remove the font to recover memory used
      
  delay(50);
 
}
