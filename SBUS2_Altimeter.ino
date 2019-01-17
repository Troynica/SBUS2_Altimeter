// This Arduino sketch reads data from an Meas MS5611 sensor and pushes
// derived telemetry data to an I2C-to-S.BUS interface. This interface
// sends it to an S.BUS2 equipped RC receiver so this telemetry data
// can be observed on the transmitter.
// 
// It also reads S.BUS data from the S.BUS2 port to detect no-signal and
// failsafe conditions to properly set the zero point.
//
// It is primarily meant as a altimeter/variometer for RC model aircraft
// but extra functionality can easily be added.
// 
// Written by Roy van der Kraan
//
// Dependencies:
//   SPI library
//   Wire library
//   SBUS library
//
// Hardware requirements:
//   Arduino compatible hardware (ATMega163p and ATMega328p tested)
//   MS5611 (device or board like GY-63)
//   I2C-to-S.BUS interface
//
//  The I2C-to-S.BUS interface I am using was developped by
//    Thomas Hedegaard Jørgensen
//    and can be obtained from here:
//    https://shop.tje.dk/catalog/product_info.php?products_id=42
//
//  The SBUS library was written by
//    Dennis Marinus
//    https://github.com/zendes/SBUS
//
//  The MS5611 library was originally written by
//    Rob Tillaart
//    https://github.com/RobTillaart/Arduino
//    modified by me for SPI use:
//    https://github.com/Troynica/MS5611

#include <SPI.h>
#include <Wire.h>
#include "SBUS.h"
#include "MS5611.h"

#define LED         3
#define SLOT_TEMP   3
#define SLOT_ALT    5
#define SLOT_VARIO  4

SBUS      sbus(Serial);
MS5611    MS5611(2);

int32_t   currentAlt, prevAlt, vario;
int32_t   alt = 0;
int16_t   Temp = 0;
uint32_t  currentMillis, prevMillis, diffMillis;
uint32_t  nextDisplay = 0;

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  Wire.begin();
  SPI.begin();
  MS5611.init();
  sbus.begin(false);
  delay(500);
  digitalWrite(LED, LOW);
}


void loop() {
  sbus.process();
  MS5611.read(12);

  if (sbus.getGoodFrames() == 0 || sbus.getFailsafeStatus() == SBUS_FAILSAFE_ACTIVE) {
    altimeterSetZero();
  }
  
  

  if (millis() >= nextDisplay) {
    //digitalWrite(LED, HIGH);
    alt=110;
    currentMillis=millis();
    nextDisplay = currentMillis + 900;
    currentAlt=alt;                         // Altitude in dm
    diffMillis=currentMillis-prevMillis;
    vario=100*(currentAlt-prevAlt);         // Alt delta in mm
    vario=100*vario/diffMillis;             // Vario in dm/s
    
    prevMillis=currentMillis;
    prevAlt=currentAlt;

    Temp=MS5611.getTemperature();
    sendTemp(SLOT_TEMP, Temp);
    sendVario(SLOT_VARIO,vario);
    sendAlt(SLOT_ALT, alt);
    //digitalWrite(LED, LOW);
  }
}




void altimeterSetZero() {
  digitalWrite(LED, HIGH);
  sendAlt(SLOT_ALT, 0);
  delay(2000);
  digitalWrite(LED, LOW);
}


void sendAlt(byte slot, int _altitude) {
  byte data;
  _altitude=(_altitude/10)+8192;
  Wire.beginTransmission(0x78);
  Wire.write(slot+32);
  data=(_altitude >> 8) | B11000000;
  Wire.write(data);
  data=_altitude & 255;
  Wire.write(data);
  Wire.endTransmission();
}


void sendVario(byte slot, int16_t _vario) {
  byte data;
  Wire.beginTransmission(0x78);
  Wire.write(slot+32);
  data=(_vario >> 8) & 255;
  Wire.write(data);
  data=_vario & 255;
  Wire.write(data);
  Wire.endTransmission();
}


void sendTemp(byte slot, int16_t _temp) {
  byte data;
  Wire.beginTransmission(0x78);
  Wire.write(slot+32);
  data=(_temp+100)%256;
  Wire.write(data);
  data=((_temp+100)/256)+128;
  Wire.write(data);
  Wire.endTransmission();
}

void clearSlots() {
  Wire.beginTransmission(0x78);
  Wire.write(0);
  Wire.write(0);
  Wire.endTransmission();
}
