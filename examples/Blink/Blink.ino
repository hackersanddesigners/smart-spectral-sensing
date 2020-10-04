
// Pin 27 turns the screen on and off 
//int BlinkPin = 27;

// Pin 4 is the red LED on the main board
int BlinkPin = 4;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BlinkPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(BlinkPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(BlinkPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
