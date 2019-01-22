# SBUS2_Altimeter
## Arduino sketch for S.BUS2 telemetry sensor for model aircraft

This Arduino sketch reads data from an Meas MS5611 sensor and pushes derived telemetry data to an I2C-to-S.BUS interface.
The interface sends the data to an S.BUS2 equipped RC transceiver which on its turn transmits the data to the handset
where it can be monitored by its operator.
It also reads S.BUS data from the S.BUS2 port to detect no-signal and failsafe conditions to properly (re)set the zero
point on the handset (otherwise the current altitude will be shown as 0). The I2C-to-S.BUS2 interface provides a signal inverter.

It is primarily meant as a altimeter/variometer for RC model aircraft but extra functionality can easily be added.

#### Written by Roy van der Kraan

### Dependencies
- SPI library
- Wire library
- MS5611 library
- SBUS library

### Hardware requirements
- Arduino compatible controller or board (ATmega168p and ATmega328p tested)
- MS5611 (device or board like GY-63)
- I2C-to-S.BUS2 interface
- S.BUS2 equipped RC set
- some additional electronics where needed (like logic level converters on data lines)

The I2C-to-S.BUS2 interface I am using was developped by **Thomas Hedegaard Jørgensen**  and can be obtained from here:
 [shop.tje.dk](https://shop.tje.dk/catalog/product_info.php?products_id=42)

The SBUS library was written by **Dennis Marinus** [GitHub](https://github.com/zendes/SBUS)

The MS5611 library was originally written by **Rob Tillaart** ([GitHub](https://github.com/RobTillaart/Arduino))
modified by me for SPI use ([GitHub](https://github.com/Troynica/MS5611)).

