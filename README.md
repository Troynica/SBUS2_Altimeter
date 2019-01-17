# SBUS2_Altimeter
Arduino sketch for S.BUS2 telemetry device for model aircraft

This Arduino sketch reads data from an Meas MS5611 sensor and pushes derived telemetry data to an I2C-to-S.BUS interface. This interface sends it to an S.BUS2 equipped RC receiver so this telemetry data can be observed on the transmitter.
It also reads S.BUS data from the S.BUS2 port to detect no-signal and failsafe conditions to properly (re)set the zero point. The I2C-to-S.BUS2 interface provides a signal inverter.

It is primarily meant as a altimeter/variometer for RC model aircraft but extra functionality can easily be added.

Written by Roy van der Kraan

Dependencies:
  SPI library
  Wire library
  SBUS library

Hardware requirements:
  Arduino compatible hardware (ATMega163p and ATMega328p tested)
  MS5611 (device or board like GY-63)
  I2C-to-S.BUS2 interface

The I2C-to-S.BUS2 interface I am using was developped by
  Thomas Hedegaard Jørgensen
  and can be obtained from here:
  https://shop.tje.dk/catalog/product_info.php?products_id=42

The SBUS library was written by
  Dennis Marinus
  https://github.com/zendes/SBUS

The MS5611 library was originally written by
  Rob Tillaart
  https://github.com/RobTillaart/Arduino
  modified by me for SPI use:
  https://github.com/Troynica/MS5611
