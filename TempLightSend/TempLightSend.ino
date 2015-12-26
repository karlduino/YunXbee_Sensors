// Copyright 2015, Matthijs Kooijman <matthijs@stdin.nl>
//
// Permission is hereby granted, free of charge, to anyone
// obtaining a copy of this document and accompanying files, to do
// whatever they want with them without any restriction, including, but
// not limited to, copying, modification and redistribution.
//
// NO WARRANTY OF ANY KIND IS PROVIDED.
//
// This example reads values from a DHT22 sensor every 10 seconds and
// sends them to the coordinator XBee, to be read by Coordinator.ino.
//
// Actually revised to use a TMP36 temperature sensor (attached to A1)
// and a regular photosensitive resistor (attached to A0)
//
// TMP36 has 47k resistor between ground and analog out
// photoresistor has 10k pull-down resistor

#include <XBee.h>
#include <Printers.h>
#include <AltSoftSerial.h>
#include "binary.h"

XBeeWithCallbacks xbee;

AltSoftSerial SoftSerial;
#define DebugSerial Serial
#define XBeeSerial SoftSerial

// TMP36 connected to A1
#define TEMP A1
// photoresistor connected to A0
#define LIGHT A0

void setup() {
  // Setup debug serial output
  DebugSerial.begin(115200);
  DebugSerial.println(F("Starting..."));

  // Setup XBee serial communication
  XBeeSerial.begin(9600);
  xbee.begin(XBeeSerial);
  delay(1);

  // Setup callbacks
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&DebugSerial);
  xbee.onResponse(printErrorCb, (uintptr_t)(Print*)&DebugSerial);

  pinMode(TEMP, INPUT);
  pinMode(LIGHT, INPUT);

  // Send a first packet right away
  sendPacket();
}

void sendPacket() {
    // Prepare the Zigbee Transmit Request API packet
    ZBTxRequest txRequest;
    txRequest.setAddress64(0x0000000000000000);

    // Allocate 9 payload bytes: 1 type byte plus two floats of 4 bytes each
    AllocBuffer<9> packet;

    // Packet type, temperature, light
    packet.append<uint8_t>(1);
    packet.append<float>(readTemperature());
    packet.append<float>(readLight());
    txRequest.setPayload(packet.head, packet.len());

    // And send it
    xbee.send(txRequest);
}

unsigned long last_tx_time = 0;

void loop() {
  // Check the serial port to see if there is a new packet available
  xbee.loop();

  // Send a packet every 10 seconds
  if (millis() - last_tx_time > 10000) {
    sendPacket();
    last_tx_time = millis();
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