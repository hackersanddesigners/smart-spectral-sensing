
// Diagnostic test for the displayed colour order
// Writen by Bodmer 17/2/19 for the TFT_eSPI library:
// https://github.com/Bodmer/TFT_eSPI
// Adapted by H&D 2020

#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// pins for the touchpad
int TP_PIN_PIN = 33;    // touch pad (button)
int TP_PWR_PIN = 25;    // power line to touch pad (pullup resistor)
bool toggleDisplay = false;   //variable to store inversion toggle
bool previous = LOW;        // variable to store previous state
long timer = 0;              // for debouncing button
long debounce = 200;        

void setup(void) {

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(0);

  //activate the pullup resistor for the touchpad
  pinMode(TP_PIN_PIN, INPUT);     // set button pin as input
  pinMode(TP_PWR_PIN, PULLUP);    // set to pull-up resistor output mode
  digitalWrite(TP_PWR_PIN, HIGH);

  //   Reduce the brightness for longer battery life
     ledcSetup(0, 1000, 8);
     ledcAttachPin(TFT_BL, 0);
     ledcWrite(0, 10);

}

void loop() {

  //read the buttonpin
  int buttonState = digitalRead(TP_PIN_PIN);
  delay(1);        // delay in between reads for stability

  if (buttonState == HIGH && previous == HIGH && millis() - timer > debounce) {   //if button is not pressed
    toggleDisplay =  !toggleDisplay;
  } 

  tft.invertDisplay( toggleDisplay ); // Where i is true or false
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 4);              // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("H&D");
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println("H&D");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("H&D");
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.println("H&D");
  delay(500);

  previous = buttonState;
}
