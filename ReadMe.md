## YunXbee_Sensors

- Arduino Uno attached to router XBee + temperature and light sensors
- Sends data to Arduino Y&uacute;n attached to coordinator XBee, which
  prints the values to console
  
Based on example in chapter 2 of 
[Wireless Sensor Networks using Arduino](https://www.packtpub.com/packtlib/book/Hardware%20&%20Creative/9781784395582)
by Matthijs Kooijman.

See also
[`YunXbee_HelloWorld`](https://github.com/karlduino/YunXbee_HelloWorld)
repository, which includes an explanation of the changes I had to make
to be able to use the Y&uacute;n.

- [`TempLight`](TempLight) is a sketch for just reading the sensor
  values and printing them to Serial
- [`TempLightYun`](TempLightYun) is similar, but for the Y&uacute;n
  (using `Console` in place of `Serial`).
- [`TempLightSend`](TempLightSend) further sends the values via the
  router XBee to be run on the Arduino Uno with XBee router
- [`TempLightReceive`](TempLightReceive) receives the values via the
  coordinator XBee, to be run on the Arduino Y&uacute;n
- [`TempLightBeebotte`](TempLightBeebotte) is like
  [`TempLightReceive`](TempLightReceive), but it also sends the input
  measurements to [Beebotte](http://beebotte.com). (Also, the coordinator
  Arduino also has temperature and light sensors.)

Later, I received three DHT22 sensors, and replaced the TMP36 ones
with those.

- [`TempHumidLight`](TempHumidLight) and
  [`TempHumidLightYun`](TempHumidLightYun) for testing the sensors
- [`TempHumidLightSend`](TempHumidLightSend) and
  [`TempHumidLightBeebotte`](TempHumidLightBeebotte) for gathering and
  sending the measurements to [Beebotte](http://beebotte.com).
