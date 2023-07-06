#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <Wire.h>
#include "BH1750.h"
#include "DHTesp.h"
#include "ThingsBoard.h"

#define UPDATE_DATA_INTERVAL 5000
#define WIFI_SSID "los"
#define WIFI_PASSWORD "12345678"
#define THINGSBOARD_ACCESS_TOKEN "UiJJL8WrweOxbEind3Hu"
#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define DHT_PIN 19
#define PIN_SDA 21
#define PIN_SCL 22
#define DHTTYPE DHTesp::DHT11



WiFiClient espClient;
ThingsBoard tb(espClient);
Ticker timerSendData;
DHTesp dht;
BH1750 lightMeter;


void WifiConnect();
void onSendSensor();

void setup()
{
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTTYPE);
  Wire.begin(PIN_SDA, PIN_SCL);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  WifiConnect();
  timerSendData.attach_ms(UPDATE_DATA_INTERVAL, onSendSensor); 
  Serial.println("System ready.");
}

void loop()
{
  if (!tb.connected())
  {
    if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN))
      Serial.println("Connected to ThingsBoard");
    else
    {
      Serial.println("Error connecting to ThingsBoard");
      delay(3000);
    }
  }
  tb.loop();
}

void onSendSensor() // Renamed from SendSensor to onSendSensor
{
  float Humid = dht.getHumidity();
  float Temp = dht.getTemperature();
  float Lux = lightMeter.readLightLevel();

  if (dht.getStatus() == DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%RH, Light: %.2f lux\n",
                   Temp, Humid, Lux);
    if (tb.connected())
    {
      tb.sendTelemetryFloat("temperature", Temp);
      tb.sendTelemetryFloat("humidity", Humid);
      tb.sendTelemetryFloat("light", Lux);
    }
  }
  else
  {
    Serial.println("DHT11 sensor error");
  }
}

void WifiConnect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
