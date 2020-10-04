int ledPin = 4;         // led is on pin 4
int TP_PIN_PIN = 33;    // touch pad (button)
int TP_PWR_PIN = 25;    // power line to touch pad (pullup resistor)

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(9600);

  pinMode(TP_PIN_PIN, INPUT);     // set button pin as input
  pinMode(TP_PWR_PIN, PULLUP);    // set to pull-up resistor output mode
  digitalWrite(TP_PWR_PIN, HIGH);

  pinMode(ledPin, OUTPUT);      // set ledPin as output
  digitalWrite(ledPin, LOW);   // turn led off to begin with
}

void loop() {

  int buttonState = digitalRead(TP_PIN_PIN);
  Serial.println(buttonState);
  delay(1);        // delay in between reads for stability

  if (buttonState == HIGH) {      //  if button is pressed 
    digitalWrite(ledPin, HIGH);   //  turn on led
  } else {
    digitalWrite(ledPin, LOW);    // else turn it off
  }
}
