#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <Wire.h>
#include <Adafruit_ADS1X15.h>   // Control del ADC
#include <ArduinoJson.h>
#include <Ticker.h>

#define MUESTRAS 50
#define ADS_ADDRESS 0x48
#define MEDIR 1
#define NO_MEDIR 0
#define FREC_MUESTREO 0.0083/8 // esta en SEGUNDOS ESTE DIVIDO DOS ESTA FALOPA
//Por Voy a tomar una fmax=60 Hz => por nyquist fs = 120 Hz => 8.33 mS



Ticker timer_medir_adc;

Adafruit_ADS1115 ads;

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
float tension_adc_2_tension_red(int16_t counts);
float tension_adc_2_corriente_red(int16_t counts);



const int capacity = JSON_ARRAY_SIZE(MUESTRAS+1) + 2*JSON_OBJECT_SIZE(MUESTRAS); // Hago espacio en memoria para el json
StaticJsonDocument<capacity> doc;

JsonObject root = doc.to<JsonObject>();

JsonArray json = root.createNestedArray("data");       // Creo el objecto Json

String output;                                    // Acá guardo la deserializacion del json
 

int a = 0;
int i = 0;
int flagAdc = MEDIR;
int adcBufferTension[MUESTRAS];
int adcBufferCorriente[MUESTRAS];
int period = 2;
unsigned long time_now = 0;

void leer_adc(){
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
  /**
     COMO ESTAN CONECTADOS EN MI PLACA (LUCHO)
     CANAL 0 -----> SESNOR I
     CANAL 1 -----> SESNOR V
  **/
  if(flagAdc==MEDIR){
    time_now += period;
    adcBufferCorriente[i] = (int)tension_adc_2_corriente_red( ads.readADC_SingleEnded(0) );
    adcBufferTension[i]   = (int)tension_adc_2_tension_red  ( ads.readADC_SingleEnded(1) );
    i++;
  }
  if(i>=MUESTRAS)  // Si lleno el buffer
    flagAdc = NO_MEDIR;
 
}

void setup(void){

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  Wire.begin();
  ads.begin();
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  
  wifiMulti.addAP("Fibertel WiFi017 2.4GHz", "0102017365");   // add Wi-Fi networks you want to connect to
  delay(10);
  Serial.println("Connecting ...");
 
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  timer_medir_adc.attach(FREC_MUESTREO, leer_adc);
}

void loop(void){
  if(flagAdc==NO_MEDIR)
    server.handleClient();    // Listen for HTTP requests from clients 
} 

void handleRoot() {
  int pepe = 0;
  for(i=0;i<MUESTRAS;i++){
    json[i]["tension"] = adcBufferTension[i];
    json[i]["corriente"] = adcBufferCorriente[i];
 
  Serial.println(adcBufferCorriente[i]);

  if( i >=48)
    pepe = json[i]["corriente"];
  }

  i=0;
  serializeJson(root, output);
  server.send(200, "text/json", output);   // Send HTTP status 200 (Ok) and send some text to the browser/client
  Serial.println(output);
  output.clear();
  flagAdc = MEDIR;
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


float tension_adc_2_corriente_red(int16_t counts){
  float mean = 2.515f;
  float fsRange = 6.144f;
  float cte_sensor = 15.15; // VER DATASHEET P.7 YO TENGO SENSOR 20A 
  //https://pdf1.alldatasheet.com/datasheet-pdf/view/168326/ALLEGRO/ACS712.html
  return (  ( ((counts * (fsRange / 32768))- mean)* cte_sensor )*1000/1000.0f );
}
float tension_adc_2_tension_red(int16_t counts){
  float mean = 2.528f;
  float fsRange = 6.144f;
  float cte_sensor = 439.01; // VER FOTO HOJA MIA
  return (  ( ((counts * (fsRange / 32768))-mean)*cte_sensor )*1000/1000.0f );
}
