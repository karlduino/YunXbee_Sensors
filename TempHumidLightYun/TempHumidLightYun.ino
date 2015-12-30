// temperature, humidity, and light sensors on Arduino Yun
//
// replaced TMP36 sensor with DHT22
//
// Still using external 5V on Yun, since its 5V was really giving 4.3V
#include <Console.h>
#include "DHT.h"

// temp and humidity connected to A1
#define DHTPIN A1
#define DHTTYPE DHT22
// photoresistor connected to A0
#define LIGHT A0

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  dht.begin();
  pinMode(LIGHT, INPUT);
  Bridge.begin();
  Console.begin();
}

void loop() {
  float light = analogRead(LIGHT)/1024.0;
    float h = dht.readHumidity();
    float t = dht.readTemperature(true);
    if(isnan(h) || isnan(t)) {
      Console.println("Failed to read from sensor.");
    }
    else {
      Console.print("Humidity: ");
      Console.print(h);
      Console.print("%    ");
      Console.print("Temp: ");
      Console.print(t);
      Console.print("*F");
    }
  Console.print("    Light reading: ");
  Console.println(light);
  delay(500);
}
