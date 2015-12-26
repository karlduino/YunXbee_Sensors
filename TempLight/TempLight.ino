// temperature and light sensors

// TMP36 connected to A1
#define TEMP A1
// photoresistor connected to A0
#define LIGHT A0

void setup() {
  pinMode(TEMP, INPUT);
  pinMode(LIGHT, INPUT);
  Serial.begin(115200);
}

void loop() {
  float voltage = analogRead(TEMP)/1024.0*5.0;
  float tempC = voltage*100.0-50.0;
  float tempF = tempC*9.0/5.0+32.0;
  float light = analogRead(LIGHT)/1024.0;
  Serial.print("Temp reading: ");
  Serial.print(tempF);
  Serial.print("    Light reading: ");
  Serial.println(light);
  delay(500);
}
