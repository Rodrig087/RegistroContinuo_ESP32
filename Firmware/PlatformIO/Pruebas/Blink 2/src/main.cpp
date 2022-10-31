#include <Arduino.h>

#define LED_BUILTIN 2

// void setup()
// {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   Serial.print("MOSI: ");
//   Serial.println(MOSI);
//   Serial.print("MISO: ");
//   Serial.println(MISO);
//   Serial.print("SCK: ");
//   Serial.println(SCK);
//   Serial.print("SS: ");
//   Serial.println(SS);
// }

// void loop()
// {
//   // put your main code here, to run repeatedly:
// }

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(250);                       // wait for a second
}