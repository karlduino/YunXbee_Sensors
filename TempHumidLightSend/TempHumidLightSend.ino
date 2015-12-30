// Copyright 2015, Matthijs Kooijman <matthijs@stdin.nl>
//
// Permission is hereby granted, free of charge, to anyone
// obtaining a copy of this document and accompanying files, to do
// whatever they want with them without any restriction, including, but
// not limited to, copying, modification and redistribution.
//
// NO WARRANTY OF ANY KIND IS PROVIDED.
//
// This example reads values from a DHT22 sensor every 5 minutes
// sends them to the coordinator XBee, to be read by Coordinator.ino.

#include <XBee.h>
#include <Printers.h>
#include <AltSoftSerial.h>
#include "DHT.h"
#include "binary.h"

XBeeWithCallbacks xbee;

AltSoftSerial SoftSerial;
#define DebugSerial Serial
#define XBeeSerial SoftSerial

// temp and humidity connected to A1
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
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

  dht.begin();
  pinMode(LIGHT, INPUT);
}

void sendPacket() {
    // Prepare the Zigbee Transmit Request API packet
    ZBTxRequest txRequest;
    txRequest.setAddress64(0x0000000000000000);

    // Allocate 13 payload bytes: 1 type byte plus three floats of 4 bytes each
    AllocBuffer<13> packet;

    // Packet type, temperature, light
    packet.append<uint8_t>(1);
    packet.append<float>(dht.readTemperature(true));
    packet.append<float>(dht.readHumidity());
    packet.append<float>(readLight());
    txRequest.setPayload(packet.head, packet.len());

    // And send it
    DebugSerial.println(F("Sending packet."));
    xbee.send(txRequest);
}

unsigned long last_tx_time = 4294567295; // largest unsigned long - 400,000
#define TIME_BETWEEN_TEMPS 300000

void loop() {
  // Check the serial port to see if there is a new packet available
  xbee.loop();

  // Send a packet every so often seconds
  if (millis() - last_tx_time > TIME_BETWEEN_TEMPS) {
    sendPacket();
    last_tx_time = millis();
  }
}

float readLight(void)
{
  float light = analogRead(LIGHT)/1024.0;
  return light;
}
