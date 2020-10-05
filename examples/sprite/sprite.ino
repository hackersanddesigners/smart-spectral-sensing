// This example plots a rotated Sprite to the screen using the pushRotated()
// function. It is written for a 240 x 320 TFT screen.

// Two rotation pivot points must be set, one for the Sprite and one for the TFT
// using setPivot(). These pivot points do not need to be within the visible screen
// or Sprite boundary.

// When the Sprite is rotated and pushed to the TFT with pushRotated(angle) it will be
// drawn so that the two pivot points coincide. This makes rotation about a point on the
// screen very simple. The rotation is clockwise with increasing angle. The angle is in
// degrees, an angle of 0 means no Sprite rotation.

// The pushRotated() function works with 1, 8 and 16 bit per pixel (bpp) Sprites.

// The original Sprite is unchanged so can be plotted again at a different angle.

// Optionally a transparent colour can be defined, pixels of this colour will
// not be plotted to the TFT.

// For 1 bpp Sprites the foreground and background colours are defined with the
// function spr.setBitmapColor(foregroundColor, backgroundColor).

// Created by Bodmer 6/1/19 as an example to the TFT_eSPI library:
// https://github.com/Bodmer/TFT_eSPI

//modified by H&D 2020

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();           // TFT object

// We will create 3 sprite objects (images) that will each appear twice

TFT_eSprite spr = TFT_eSprite(&tft); // Sprite object
TFT_eSprite spr2 = TFT_eSprite(&tft); // Sprite object
TFT_eSprite spr3 = TFT_eSprite(&tft); // Sprite object
TFT_eSprite spr4 = TFT_eSprite(&tft); // Sprite object

// =======================================================================================
// Setup
// =======================================================================================

void setup()   {
  Serial.begin(250000); // Debug only

  tft.begin();  // initialize
  tft.setRotation(1);
}

// =======================================================================================
// Loop
// =======================================================================================

void loop() {
  
  int xw = tft.width()/2;   // xw, yh is +/- midle of screen
  int yh = tft.height()/2+10;

  showMessage("BODYBUILDING.BODYBUILDING");
  tft.setPivot(xw, yh);     // Set pivot (indicated by cross) 
  drawX(xw, yh);            // take values above, draw cross with function below

  // Create the Sprite
  spr.setColorDepth(8);      // Create an 8bpp Sprite of 60x30 pixels
  spr.createSprite(30, 20);  // 8bpp requires 64 * 30 = 1920 bytes > the black box
  spr.setPivot(0, 25);      // define pivot distance relative to top left corner of Sprite
  spr.fillSprite(TFT_BLACK); // Fill the Sprite with black

  spr.setTextColor(TFT_GREEN);        // Green text
  spr.setTextDatum(MC_DATUM);         // Middle centre datum
  spr.drawString("H&D", 15, 9, 2); // Plot text, in Sprite at 15, 7, in font 2

  // Create the Sprite number 2
  spr2.setColorDepth(8);      // Create an 8bpp Sprite of 60x30 pixels
  spr2.createSprite(30, 20);  // 8bpp requires 64 * 30 = 1920 bytes > the black box
  spr2.setPivot(40, 25);      // define pivot distance relative to top left corner of Sprite
  spr2.fillSprite(TFT_BLACK); // Fill the Sprite with black

  spr2.setTextColor(TFT_GREEN);        // Green text
  spr2.setTextDatum(MC_DATUM);         // Middle centre datum
  spr2.drawString("H&D", 15, 9, 2); // Plot text, in Sprite at 15, 7, in font 2

  tft.fillRect(0, 20, tft.width(), tft.height()-20, TFT_YELLOW);

  spr.pushRotated(0);
  spr.pushRotated(180);
  spr2.pushRotated(0);
  spr2.pushRotated(180);

  delay(2000);

  tft.fillRect(0, 20, tft.width(), tft.height()-20, TFT_YELLOW);

  spr.pushRotated(270);
  spr.pushRotated(90);
  spr2.pushRotated(270);
  spr2.pushRotated(90);
 
  delay(2000);  // Pause so we see it

}

// =======================================================================================
// Draw an X centered on x,y
// =======================================================================================

void drawX(int x, int y)
{
  tft.drawLine(x-5, y-5, x+5, y+5, TFT_RED);
  tft.drawLine(x-5, y+5, x+5, y-5, TFT_RED);
}

// =======================================================================================
// Show a message at the top of the screen
// =======================================================================================

void showMessage(String msg)
{
  // Clear the screen areas
  tft.fillRect(0, 0, tft.width(), 20, TFT_BLACK);
  tft.fillRect(0, 20, tft.width(), tft.height()-20, TFT_YELLOW);

  uint8_t td = tft.getTextDatum(); // Get current datum

  tft.setTextDatum(TL_DATUM);      // Set new datum

  tft.drawString(msg, 5, 5, 1); // Message in font 2

  tft.setTextDatum(td); // Restore old datum
}

// =======================================================================================
