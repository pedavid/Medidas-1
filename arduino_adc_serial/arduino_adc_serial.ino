#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define MUESTRAS 500
#define ADS_ADDRESS 0x48

int a = 0;
int i = 0;
int i_adc[MUESTRAS];
int period = 2;
unsigned long time_now = 0;

Adafruit_ADS1115 ads(ADS_ADDRESS);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  ads.begin();
  // put your setup code here, to run once:

}

void loop() {
  if(millis() >= time_now + period){
    time_now += period;
    i_adc[i] = ads.readADC_SingleEnded(0);
    i++;
  }
  if(i>=MUESTRAS && a == 0){
    for(i=0;i<MUESTRAS;i++){
      Serial.println(i_adc[i]);
    }
    i=0;
    a=1;
  }
  // put your main code here, to run repeatedly:

}
