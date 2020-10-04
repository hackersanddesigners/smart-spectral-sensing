
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

// Maximum number of generations until the screen is refreshed
#define MAX_GEN_COUNT 500

//screen orientation: 0 = portrait, add 1 to rotate clockwise by 90 degrees (0,1,2,3)
int orientation = 1;    // for landscape

// pins for the touchpad
int TP_PIN_PIN = 33;    // touch pad (button)
int TP_PWR_PIN = 25;    // power line to touch pad (pullup resistor)
int screenPin = 27;     // backlight of the screen
// pins for backlight of screen

void setup() {

  //Set up the display
  tft.init();
  tft.setRotation(orientation);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(screenPin, OUTPUT);

  //activate the pullup resistor for the touchpad
  pinMode(TP_PIN_PIN, INPUT);     // set button pin as input
  pinMode(TP_PWR_PIN, PULLUP);    // set to pull-up resistor output mode
  digitalWrite(TP_PWR_PIN, HIGH);

}

void loop() {

  //read the buttonpin
  int buttonState = digitalRead(TP_PIN_PIN);
  delay(1);        // delay in between reads for stability

  if (buttonState == HIGH) {      //  if button is pressed
    digitalWrite(screenPin, HIGH);   // turn the display off)
    
    tft.fillScreen(TFT_VIOLET);        //set background color, specified in TFT_eSPI.h file
    tft.setTextColor(TFT_GREENYELLOW);
    tft.setTextSize(3);                //text size
    tft.setCursor(tft.width()/2-40, tft.height()/2-10);      // x,y where the word is drawn
    tft.println(F("hello"));
  } else {
    digitalWrite(screenPin, LOW);   // turn the display off)
  }

  delay(50);

}

