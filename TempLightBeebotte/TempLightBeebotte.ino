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
#include <Console.h>

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
long last_temp_time = 0;
#define TIME_BETWEEN_TEMPS 10000

void setup() {
  // Setup debug serial output
  Bridge.begin();
  Console.begin();
  while(!Console); // wait for console to open
  DebugSerial.println(F("Starting..."));

  // Setup XBee serial communication
  XBeeSerial.begin(9600);
  xbee.begin(XBeeSerial);
  delay(1);

  // Setup callbacks
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onResponse(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onZBExplicitRxResponse(processRxPacket);
}

}


void processRxPacket(ZBExplicitRxResponse& rx, uintptr_t) {
  Buffer b(rx.getData(), rx.getDataLength());
  uint8_t type = b.remove<uint8_t>();

  if (type == 1 && b.len() == 8) {
    DebugSerial.print(F("Temp/Light (explicit) packet received from "));
    printHex(DebugSerial, rx.getRemoteAddress64());
    DebugSerial.println();
    DebugSerial.print(F("Temperature: "));
    DebugSerial.println(b.remove<float>());
    DebugSerial.print(F("Light: "));
    DebugSerial.println(b.remove<float>());
    return;
  }

  DebugSerial.println(F("Unknown or invalid packet"));
  printResponse(rx, DebugSerial);
}



void loop() {
  // Check the serial port to see if there is a new packet available
  xbee.loop();

  if(millis() - last_temp_time > TIME_BETWEEN_TEMPS) {
    last_temp_time = millis();
    float temp = readTemperature();
    float light = readLight();
    DebugSerial.print(F("Temperature: "));
    DebugSerial.print(temp);
    DebugSerial.print(F("    "));
    DebugSerial.print(F("Light: "));
    DebugSerial.println(light);
  }
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