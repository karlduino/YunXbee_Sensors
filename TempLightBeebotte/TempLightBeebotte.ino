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
// This example receives packets containing temperature and light
// data, as sent by the TempLightSend.ino sketch, and prints their contents.
//
// Modified to work with an Arduino Yun, printing the results to Console.
// Using a Sparkfun Xbee shield, with pins 0-3 of Yun
// not connected to the shield, and with shield pins 2 & 13 connected, and also
// shield pins 3 and 5 connected (so that 13 is Rx, and 5 is Tx)
//
// Actually revised to use a TMP36 temperature sensor (attached to A1)
// and a regular photosensitive resistor (attached to A0), with 10k
// pull-down resistor


#include <XBee.h>
#include <Printers.h>
#include <AltSoftSerial.h>
#include "binary.h"
#include <Bridge.h>
#include <Console.h>
#include <YunClient.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "private.h"

YunClient client;

XBeeWithCallbacks xbee;

AltSoftSerial SoftSerial;
#define DebugSerial Console
#define XBeeSerial SoftSerial

#define ALTSS_USE_TIMER3
#define INPUT_CAPTURE_PIN      13 // receive
#define OUTPUT_COMPARE_A_PIN    5 // transmit

// TMP36 connected to A1
#define TEMP A1
// photoresistor connected to A0
#define LIGHT A0
unsigned long last_temp_time = 4294767295; // largest unsigned long - 200,000
#define TIME_BETWEEN_TEMPS 180000

// beebotte stuff
const char MQTT_SERVER[] PROGMEM    = "mqtt.beebotte.com";
const char MQTT_CLIENTID[] PROGMEM  = "";
const char MQTT_USERNAME[] PROGMEM  = MY_BEEBOTTE_KEY; // defined in private.h
const char MQTT_PASSWORD[] PROGMEM  = "";
const int MQTT_PORT = 1883;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_CLIENTID,
                          MQTT_USERNAME, MQTT_PASSWORD);

void setup() {
  // Setup debug serial output
  Bridge.begin();
  Console.begin();

  // Setup XBee serial communication
  XBeeSerial.begin(9600);
  xbee.begin(XBeeSerial);
  delay(1);

  // Setup callbacks
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onResponse(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onZBExplicitRxResponse(processRxPacket);
}

void loop() {
  if(millis() - last_temp_time > TIME_BETWEEN_TEMPS) {
    last_temp_time = millis();
    publish(F("basement/temperature"), readTemperature());
    publish(F("basement/light"), readLight());
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
  if (type == 1 && b.len() == 8) {
    if(addr == BEDROOM_XBEE_ADDR) { // defined in private.h
      publish(F("bedroom/temperature"), b.remove<float>());
      publish(F("bedroom/light"), b.remove<float>());
      return;
    }
    if(addr == LIVINGROOM_XBEE_ADDR) { // defined in private.h
      publish(F("livingroom/temperature"), b.remove<float>());
      publish(F("livingroom/light"), b.remove<float>());
      return;
    }
    DebugSerial.println(F("Unknown address"));
    return;
  }

  DebugSerial.println(F("Unknown or invalid packet"));
  printResponse(rx, DebugSerial);
}



float readTemperature(void)
{
  float voltage = analogRead(TEMP)/1024.0*5.0;
  float tempC = voltage*100.0-50.0;
  float tempF = tempC*9.0/5.0+32.0;
  return tempF;
}

float readLight(void)
{
  float light = analogRead(LIGHT)/1024.0;
  return light;
}
