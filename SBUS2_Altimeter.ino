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
//
//
//  The altitude calculation is an approximate. I use a linear function with
//  correction to calculate the altitude from the barometric pressure. This
//  is within a few meters accurate up to 500 meters. Enough precision for
//  model aircraft. The function doing this caculation is called:
//  calcAltitudeInt()
//
//  Precision is measured against the (already simplified) common formula:
//     Alt = 44330 * (1 - (P/P0)^0.190295 );
//  I also put this formula as a function in the sketch: calcAltitudeFloat()
//
//  The output difference for altitudes up to 300m is less than half a meter,
//  for altitudes up to 700m, it is ~1m, for altitudes up to 1000m, it is ~5m.
//    (calculated for a location at sea level with P0 varying from 100000
//    to 103000 Pa).


#include <SPI.h>
#include <Wire.h>
#include "SBUS.h"
#include "MS5611.h"

#define LED         3
#define BLED        5
#define FSLED       4
#define SLOT_TEMP   3
#define SLOT_ALT    2
#define SLOT_VARIO  1

SBUS      sbus(Serial);
MS5611    MS5611(2);

#define CONST1 61
uint64_t  const0;
uint32_t  p0;
int32_t   currentAlt, prevAlt, vario;
int32_t   alt = 0;
int16_t   Temp = 0;
uint32_t  currentMillis, prevMillis, diffMillis;
uint32_t  nextDisplay = 0;

void setup() {
  sendAlt(SLOT_ALT, 0);
  pinMode(LED, OUTPUT);
  pinMode(BLED, OUTPUT);
  pinMode(FSLED, OUTPUT);
  digitalWrite(LED, HIGH);
  digitalWrite(FSLED, HIGH);
  Wire.begin();
  SPI.begin();
  MS5611.init();
  sbus.begin(false);

  // take average of 4 measurements for p0 :
  MS5611.read(12);
  delay(300);
  p0  = MS5611.getPressure();
  MS5611.read(12);
  delay(300);
  p0 += MS5611.getPressure();
  MS5611.read(12);
  delay(300);
  p0 += MS5611.getPressure();
  MS5611.read(12);
  delay(300);
  p0 += MS5611.getPressure();
  p0 = p0 >> 2;  // divide by 4
  
  const0  = (uint64_t)p0*1400ULL;
  const0 /= 23ULL;
  const0 += 13503913ULL;
  digitalWrite(LED, LOW);
}


void loop() {
  sbus.process();
  if (sbus.getGoodFrames() < 100 || sbus.getFailsafeStatus()) {
    digitalWrite(FSLED, (boolean)(millis()&256));
    sendAlt(SLOT_ALT, 0);
    while (sbus.getGoodFrames() < 100 || sbus.getFailsafeStatus()) {
      digitalWrite(FSLED, (boolean)(millis()&256));
      sbus.process();
    }
    digitalWrite(FSLED, LOW);
    delay(2000);
  }
  if (millis() >= nextDisplay) {
    nextDisplay = currentMillis + 500;
    digitalWrite(BLED, HIGH);
    MS5611.read(12);
    currentMillis=millis();
    alt=calcAltitudeInt(MS5611.getPressure(),p0);
    currentAlt=alt;                         // Altitude in cm
    diffMillis=currentMillis-prevMillis;
    vario  = currentAlt-prevAlt;            // Alt delta in cm
    vario  = vario < 1;                     // multiply by 2 (500ms)

    prevMillis = currentMillis;
    prevAlt    = currentAlt;

    alt /= 10L;                             // Alt cm -> dm
    Temp = MS5611.getTemperature() / 100;
    sendTemp(SLOT_TEMP, Temp);
    sendVario(SLOT_VARIO,vario);
    sendAlt(SLOT_ALT,alt);
    digitalWrite(BLED, LOW);
  }
}


int32_t calcAltitudeFloat(int32_t _p, int32_t _p0) {
  float _ac  = (float)44330 * (1 - pow(((float) _p/_p0), 0.190295));
  return (int32_t)_ac;
}

int32_t calcAltitudeInt(uint64_t _p, uint64_t _p0) {
  int64_t _ac;
    _ac  = const0;
    _ac  += (_p0-_p)*CONST1;
    _ac *= _p;
    _ac /= _p0;
    _ac  = const0 - _ac;
    _ac  = _ac >> 4;
  
   return (int32_t)(_ac);
}



void sendAlt(byte slot, int _altitude) {
  byte data;
  _altitude /= 10;
  _altitude += 8192;
  data       = (_altitude >> 8) | B11000000;
  Wire.beginTransmission(0x78);
  Wire.write(slot+32);
  Wire.write(data);
  data = _altitude & 255;
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
