// Copyright 2015, Matthijs Kooijman <matthijs@stdin.nl>
//
// Permission is hereby granted, free of charge, to anyone
// obtaining a copy of this document and accompanying files, to do
// whatever they want with them without any restriction, including, but
// not limited to, copying, modification and redistribution.
//
// NO WARRANTY OF ANY KIND IS PROVIDED.
//
//
// This example receives packets containing temperature, humidity, and light
// data, as sent by the TempHumidLightSend.ino sketch, and prints their contents.
//
// Modified to work with an Arduino Yun, printing the results to Console.
// Xbee connected directly to Yun, with Rx to 13 and Tx to 5
//
// DHT22 connected to A1
// photoresistor connected to A0

#include <XBee.h>
#include <Printers.h>
#include <AltSoftSerial.h>
#include "binary.h"
#include <Bridge.h>
#include <Console.h>
#include <YunClient.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "DHT.h"
#include "private.h"

YunClient client;

XBeeWithCallbacks xbee;

AltSoftSerial SoftSerial;
#define DebugSerial Console
#define XBeeSerial SoftSerial

#define ALTSS_USE_TIMER3
#define INPUT_CAPTURE_PIN      13 // receive
#define OUTPUT_COMPARE_A_PIN    5 // transmit

// temp and humidity connected to A1
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
// photoresistor connected to A0
#define LIGHT A0
unsigned long last_temp_time = 4294567295; // largest unsigned long - 400,000
#define TIME_BETWEEN_TEMPS 300000 // 5 min

// beebotte stuff
const char MQTT_SERVER[] PROGMEM    = "mqtt.beebotte.com";
const char MQTT_CLIENTID[] PROGMEM  = "";
const char MQTT_USERNAME[] PROGMEM  = MY_BEEBOTTE_KEY; // defined in private.h
const char MQTT_PASSWORD[] PROGMEM  = "";
const int MQTT_PORT = 1883;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_CLIENTID,
                          MQTT_USERNAME, MQTT_PASSWORD);

String logfile = "/mnt/sd/sensor_readings.csv";

void setup() {
  // Setup debug serial output
  Bridge.begin();
  Console.begin();

  // Setup XBee serial communication
  XBeeSerial.begin(9600);
  xbee.begin(XBeeSerial);
  delay(1);

  pinMode(LIGHT, INPUT);
  dht.begin();

  // Setup callbacks
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onResponse(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onZBExplicitRxResponse(processRxPacket);
}

void loop() {
  float temperature, humidity, light;
  if(millis() - last_temp_time > TIME_BETWEEN_TEMPS) {
    last_temp_time = millis();

    temperature = dht.readTemperature(true);
    humidity = dht.readHumidity();
    light = readLight();

    // write to file
    writeToFile(logfile, "basement", temperature, humidity, light);

    // publish to beebotte
    publish(F("basement/temperature"), temperature);
    publish(F("basement/humidity"), humidity);
    publish(F("basement/light"), light);
  }

  // Check the serial port to see if there is a new packet available
  xbee.loop();
}

void publish(const __FlashStringHelper *resource, float value) {
  // Use JSON to wrap the data, so Beebotte will remember the data
  // (instead of just publishing it to whoever is currently 
  // listening).
  String data;
  data += "{\"data\": ";
  data += value;
  data += ", \"write\": true}";
  DebugSerial.print(F("Publishing "));
  DebugSerial.print(data);
  DebugSerial.print(F( " to "));
  DebugSerial.println(resource);
  // Publish data and try to reconnect when publishing data fails
  if (!mqtt.publish(resource, data.c_str())) {
    DebugSerial.println(F("Failed to publish, trying reconnect..."));
    connect();
    if (!mqtt.publish(resource, data.c_str()))
      DebugSerial.println(F("Still failed to publish data"));
  }
}

void connect() {
  client.stop(); // Ensure any old connection is closed
  uint8_t ret = mqtt.connect();
  if (ret == 0)
    DebugSerial.println(F("MQTT connected"));
  else
    DebugSerial.println(mqtt.connectErrorString(ret));
}

void processRxPacket(ZBExplicitRxResponse& rx, uintptr_t) {
  Buffer b(rx.getData(), rx.getDataLength());
  uint8_t type = b.remove<uint8_t>();
  XBeeAddress64 addr = rx.getRemoteAddress64();
  DebugSerial.print(F("Temp/Light (explicit) packet received from "));
  printHex(DebugSerial, addr);
  DebugSerial.println();
  float temperature, humidity, light;
  if (type == 1 && b.len() == 12) {
    temperature = b.remove<float>();
    humidity = b.remove<float>();
    light = b.remove<float>();

    if(addr == BEDROOM_XBEE_ADDR) { // defined in private.h
      writeToFile(logfile, "bedroom", temperature, humidity, light);
      publish(F("bedroom/temperature"), temperature);
      publish(F("bedroom/humidity"), humidity);
      publish(F("bedroom/light"), light);
      return;
    }
    if(addr == LIVINGROOM_XBEE_ADDR) { // defined in private.h
      writeToFile(logfile, "livingroom", temperature, humidity, light);
      publish(F("livingroom/temperature"), temperature);
      publish(F("livingroom/humidity"), humidity);
      publish(F("livingroom/light"), light);
      return;
    }
    DebugSerial.println(F("Unknown address"));
    return;
  }

  DebugSerial.println(F("Unknown or invalid packet"));
  printResponse(rx, DebugSerial);
}

float readLight(void)
{
  float light = analogRead(LIGHT)/1024.0;
  return light;
}

float writeToFile(String filename, String location,
                  float temperature, float humidity, float light)
{
  if(!FileSystem.exists(filename)) { // file doesn't exist yet
    writeHeader(filename);
  }
  
  String dataString;
  dataString += getTimeStamp();
  dataString += "," + location;
  dataString += "," + String(temperature);
  dataString += "," + String(humidity);
  dataString += "," + String(light);

  File dataFile = FileSystem.open(filename, FILE_APPEND);
  dataFile.println(dataString);
  dataFile.close();
}

// create file and write header
float writeHeader(String filename)
{
  File dataFile = FileSystem.open(filename, FILE_WRITE);
  dataFile.println("date,time,location,temperature,humidity,light");
  dataFile.close();
}

// get date and time, with comma between
String getTimeStamp() {
  String result;
  Process time;
  time.begin("date");
  time.addParameter("+%Y-%m-%d,%T"); // format like 2015-12-31,23:15:25
  time.run();

  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }

  return result;
}
