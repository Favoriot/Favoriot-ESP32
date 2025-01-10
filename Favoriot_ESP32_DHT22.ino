/*
 * Board: Node32 Lite (ESP32 Dev Module)
 * https://www.cytron.io/p-node32-lite-wifi-and-amp;-bluetooth-development-kit
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SimpleDHT.h>

#define APIKEY  "rw-apikey/accesstoken"                        //replace API key
#define DEVICE_DEV_ID "deviceDefault@favoriot"      //replace device developer id

const int SEN_PIN = 19;                               // Output Pin GPIO19
SimpleDHT22 dht22(SEN_PIN);

const char ssid[] = "Your wifi SSID";                 //replace wifi SSID 
const char password[] = "Your Wifi password";         //replace wifi password 

float temperature = 0;
float humidity = 0;
long previousMillis = 0;
int interval = 5000; // 5 seconds

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if (millis() - previousMillis > interval) {
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT22 failed, err="); Serial.println(err);
      delay(1000);
      return;
    }
  
    Serial.print("Sample OK: ");
    Serial.print((float)temperature);
    Serial.print(" *C, ");
    Serial.print((float)humidity);
    Serial.println(" RH%");
    
    StaticJsonDocument<200> doc;
  
    JsonObject root = doc.to<JsonObject>(); // Json Object refer to { }
    root["device_developer_id"] = DEVICE_DEV_ID;
  
    JsonObject data = root.createNestedObject("data");
    data["temperature"] = (int)temperature;
    data["humidity"] = (int)humidity;
  
    String body;
    serializeJson(root, body);
    Serial.println(body);
  
    HTTPClient http;
  
    http.begin("https://apiv2.favoriot.com/v2/streams");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Apikey", APIKEY);
  
    int httpCode = http.POST(body);
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    previousMillis = millis();
  }
}
