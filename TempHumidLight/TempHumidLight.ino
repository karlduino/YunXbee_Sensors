// temperature, humidity, and light sensors on Arduino Uno
//
// replaced TMP36 sensor with DHT22

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
  Serial.begin(115200);
}

void loop() {
  float light = analogRead(LIGHT)/1024.0;
    float h = dht.readHumidity();
    float t = dht.readTemperature(true);
    if(isnan(h) || isnan(t)) {
      Serial.println("Failed to read from sensor.");
    }
    else {
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print("%    ");
      Serial.print("Temp: ");
      Serial.print(t);
      Serial.print("*F");
    }
  Serial.print("    Light reading: ");
  Serial.println(light);
  delay(500);
}
