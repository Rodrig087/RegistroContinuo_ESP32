#include <Arduino.h>

#define P1 15
#define LedTest 2

void IRAM_ATTR toggleLED()
{
  digitalWrite(LedTest, !digitalRead(LedTest));
}

void setup()
{
  pinMode(LedTest, OUTPUT);
  pinMode(P1, INPUT);
  attachInterrupt(P1, toggleLED, RISING);
}

void loop()
{
  // put your main code here, to run repeatedly:
}