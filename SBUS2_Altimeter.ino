#include "MS5611.h"
#include <SPI.h>
#include <Wire.h>
#include <SBUS.h>

#define LED_BUILTIN 3
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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Wire.begin();
  SPI.begin();
  MS5611.init();
  sbus.begin(false);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}


void loop() {
  sbus.process();
  MS5611.read(12);

  if (sbus.getGoodFrames() == 0 || sbus.getFailsafeStatus() == SBUS_FAILSAFE_ACTIVE) {
    altimeterSetZero();
  }
  
  

  if (millis() >= nextDisplay) {
    //digitalWrite(LED_BUILTIN, HIGH);
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
    //digitalWrite(LED_BUILTIN, LOW);
  }
}




void altimeterSetZero() {
  digitalWrite(LED_BUILTIN, HIGH);
  sendAlt(SLOT_ALT, 0);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
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
