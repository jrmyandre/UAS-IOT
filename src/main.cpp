//Jeremy Andre Muljono 2540128184 UAS IOT 2022 Even Semester

//import library yang akan dipakai
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "BH1750.h"
#include "DHTesp.h"
#include "ThingsBoard.h"

//menentukan pin yang akan digunakan
#define PIN_DHT 19
#define PIN_SCL 22
#define PIN_SDA 23

//deklarasi ssid dan password wifi
const char* ssid = "metallica";
const char* password = "jlmerak6";

//dekalarasi thingsboard server dan access token
const char* thingsboardServer = "thingsboard.cloud";
const char* thingsboardAccessToken = "QtmuHTl2h77y3mnD270M";


//deklarasi variabel
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);
DHTesp dht;
BH1750 lightMeter;
float temperature;
float humidity;
float lux;


//deklarasi functions
void WifiConnect();
void taskSendData(void* arg);


//setup
void setup() {
//menentukan kecepatan menerima data dari serial port
Serial.begin(9600);

//initialisasi sensor
dht.setup(PIN_DHT, DHTesp::DHT11);
Wire.begin(PIN_SDA, PIN_SCL);
lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);

//call function untuk konek ke wifi dan mqtt
WifiConnect();

//pembuatan tasks serta memorinya dan lokasi prosesornya
xTaskCreatePinnedToCore(taskSendData, "taskSendData", 2048, NULL, 1, NULL, 0);
}

//main program
void loop() {
  if (!tb.connected()) {
    if (!tb.connect(thingsboardServer, thingsboardAccessToken)) {
      Serial.println("Failed to connect");
    }
    else{
      Serial.println("Connected to Thingsboard");
      delay(2000);
    }
  }
  tb.loop();

}


//fucntion untuk get data dari sensor lalu mengirimkan ke thingsboard
void taskSendData(void* arg){
  for(;;){
    temperature = dht.getTemperature();
    humidity = dht.getHumidity();
    lux = lightMeter.readLightLevel();
    if (dht.getStatus()==DHTesp::ERROR_NONE)
    {
      Serial.printf("Temperature: %.2f C, Humidity: %.2f %%, Light: %.2f lx\n", 
      temperature, humidity, lux);

      if (tb.connected()) {
        tb.sendTelemetryFloat("temperature", temperature);
        tb.sendTelemetryFloat("humidity", humidity);

      }
      
    }
    else{
      Serial.printf("Light: %.2f lx\n", lux);
      Serial.printf("Failed to get temperature and humidity\n");
      }
    tb.sendTelemetryFloat("light", lux);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}


//function untuk konek ke wifi
void WifiConnect()
{
WiFi.begin(ssid, password);//konek ke ssid dan password sesuai yang kita tentukan

while (WiFi.waitForConnectResult() != WL_CONNECTED) {
Serial.println("Connection Failed! Rebooting...");
delay(5000);
ESP.restart();
}//apabila tidak bisa konek ke wifi, restart device setelah 5 detik

Serial.print("System connected with IP address: ");
Serial.println(WiFi.localIP());
Serial.printf("RSSI: %d\n", WiFi.RSSI());//ketika terkonek, serial print IP address dan RSSI
}