// temperature and light sensors on Arduino Yun
//
// was getting terrible temperature measurements from the Arduino Yun
// using the TMP36 sensor
//
// 5V on Yun really gives about 4.3V
// So: using external power (breadboard power supply from SparkFun,
// which gives about 4.96V)
// Need to connect ground to Arduino ground
// Also need to connect external power to arduino's AREF, I think
#include <Console.h>

// TMP36 connected to A1
#define TEMP A1
// photoresistor connected to A0
#define LIGHT A0

void setup() {
  pinMode(TEMP, INPUT);
  pinMode(LIGHT, INPUT);
  Bridge.begin();
  Console.begin();
}

void loop() {
  float voltage = analogRead(TEMP)/1024.0*5.0;
  float tempC = voltage*100.0-50.0;
  float tempF = tempC*9.0/5.0+32.0;
  float light = analogRead(LIGHT)/1024.0;
  Console.print("Temp reading: ");
  Console.print(tempF);
  Console.print("    Light reading: ");
  Console.println(light);
  delay(500);
}
