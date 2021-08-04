#include <Wire.h>
#include <Adafruit_ADS1015.h>
//***
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;
//***

#define ACOPLE_MUZZI 2.5
#define MUESTRAS 100

Adafruit_ADS1115 ads;
float vectorV[MUESTRAS], vectorI[MUESTRAS];
float v_rms, i_rms, p_rms, p_act, cos_phi;
unsigned long time_now = 0;
int period =2; //mSeg
int i=0;

#define N 16
float buf[N];
int ix;
float acc, vrms_acc;

void setup(void) {
  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Callibri10);
  oled.clear();
  
  Serial.begin(115200);
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();

  inicializar_promedio_movil_2 (buf, &ix, N, &acc);;
}

void loop(void) {
  if(millis() >= time_now + period){
    time_now += period;
    vectorV[i] = (((float)ads.readADC_SingleEnded(2)*0.1875/1000) - 2.5)*30/0.172;
    vectorI[i] = (((float)ads.readADC_SingleEnded(0)*0.1875/1000) - 2.51)*1/0.185; //acs712 0A->2.51V
    i++;
  }
  
  if(i >= MUESTRAS){
    for(i=0;i<MUESTRAS;i++){
      
      v_rms += vectorV[i] * vectorV[i];  
      i_rms += vectorI[i] * vectorI[i]; 
      p_act += vectorV[i] * vectorI[i];
 
    }
      v_rms /= MUESTRAS;
      v_rms = sqrt(v_rms);
      //Serial.println(v_rms);
      //Serial.println(((int)(v_rms*100)));
      v_rms = (float)((int)(v_rms*100))/100;

      vrms_acc = promedio_movil_2 (v_rms, buf, &ix, N, &acc);
      
      //Serial.println(v_rms);
      
      i_rms /= MUESTRAS;
      i_rms = sqrt(i_rms);
      //Serial.println(i_rms);
      //Serial.println(((int)(i_rms*100)));
      i_rms = (float)((int)(i_rms*100))/100;
      //Serial.println(i_rms);
            
      p_rms = vrms_acc*i_rms;
      p_act = p_act/MUESTRAS;
      cos_phi = p_act/p_rms;

      

    oled.setCursor(0, 0 );
    oled.print("v_rms: ");
    oled.println(vrms_acc);
    oled.print("i_rms: ");
    oled.println(i_rms);
    oled.print("p_rms: ");
    oled.println(p_rms);
    oled.print("p_act: ");
    oled.println(p_act);
    //oled.print("cos phi: ");
    //oled.println(cos_phi);
      i=0;

    //while(1){}
  }
 
}

void inicializar_promedio_movil_2 (float *buffer, int *ix, int len, float *acc)
{
  int i;
  *ix = 0;
  *acc = 0;
  for (i = 0; i < len; i++)
    buffer[i] = 0;
}

float promedio_movil_2 (float x, float *buffer, int *ix, int len, float *acc)
{
  *acc += x - buffer[*ix];
  buffer[*ix] = x;
  if (++*ix == len)
    *ix = 0;
  return *acc / len;
}
