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
#define OUTPUT_COMPARE_A_PIN        5 // transmit

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
  xbee.onZBRxResponse(processRxPacket);
}

void processRxPacket(ZBRxResponse& rx, uintptr_t) {
  Buffer b(rx.getData(), rx.getDataLength());
  uint8_t type = b.remove<uint8_t>();

  if (type == 1 && b.len() == 8) {
    DebugSerial.print(F("Temp/Light packet received from "));
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
}
