#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <Wire.h>
#include <Adafruit_ADS1X15.h>   // Control del ADC
#include <ArduinoJson.h>
#include <Ticker.h>

Ticker timer_medir_adc;

Adafruit_ADS1115 ads;

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

#define MUESTRAS 50
#define ADS_ADDRESS 0x48
#define MEDIR 1
#define NO_MEDIR 0
#define FREC_MUESTREO 0.0083/8 // esta en SEGUNDOS ESTE DIVIDO DOS ESTA FALOPA
                               


//Por Voy a tomar una fmax=60 Hz => por nyquist fs = 120 Hz => 8.33 mS

const int capacity = JSON_ARRAY_SIZE(MUESTRAS) + 2*JSON_OBJECT_SIZE(MUESTRAS); // Hago espacio en memoria para el json
StaticJsonDocument<capacity> doc;

JsonObject json = doc.createNestedObject();       // Creo el objecto Json

String output;                                    // Acá guardo la deserializacion del json
 

int a = 0;
int i = 0;
int flagAdc = MEDIR;
int adcBuffer[MUESTRAS];
int period = 2;
unsigned long time_now = 0;

void leer_adc(){
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
   
  if(flagAdc==MEDIR){
    time_now += period;
    adcBuffer[i] = ads.readADC_SingleEnded(0);  //bufferTension
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

  wifiMulti.addAP("Pedro_2.4GHz", "ViceCityFever142024");   // add Wi-Fi networks you want to connect to

  Serial.println("Connecting ...");
  int i = 0;
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
  if(flagAdc==NO_MEDIR){
    server.handleClient();    // Listen for HTTP requests from clients
  }
} 

void handleRoot() {
  for(i=0;i<MUESTRAS;i++){
     Serial.println(adcBuffer[i]);
    doc[i]["tension"] = adcBuffer[i];
    doc[i]["corriente"] = i;
  }
  i=0;
  serializeJson(doc, output);
  server.send(200, "text/json", output);   // Send HTTP status 200 (Ok) and send some text to the browser/client
  flagAdc = MEDIR;
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
