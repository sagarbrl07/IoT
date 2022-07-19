#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
const char* ssid = "POCO F3";
const char* password = "888888888";
#define DHTPIN 5
#define DHTTYPE    DHT11
float humidity, temperature;
DHT dht(DHTPIN, DHTTYPE);
StaticJsonDocument<50> doc;
StaticJsonDocument<64> doc1;
JsonObject object;
int relayInput = 4;
const char* serverName = "http://192.168.177.157:5005/greenhouse_api/post_data";
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(relayInput, OUTPUT);
  object = doc.to<JsonObject>();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      temperature = dht.readTemperature();
      humidity = dht.readHumidity();
      Serial.println(temperature);
      Serial.println(humidity);
      object["temperature"] = temperature;
      object["humidity"] = humidity;
      serializeJson(doc, Serial);
      Serial.println("");
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
      String requestBody;
      serializeJson(doc, requestBody);
      int httpResponseCode = http.POST(requestBody);
      if (httpResponseCode > 0) {
        String response = http.getString();
        DeserializationError error = deserializeJson(doc1, response);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        bool relay1 = doc1["relay1"];
        bool relay2 = doc1["relay2"];
        if (relay1 == 1) {
          digitalWrite(relayInput, LOW);
        }
        else {
          digitalWrite(relayInput, HIGH);
        }
        Serial.println(httpResponseCode);
        Serial.println(response);
        Serial.println(relay1);
        Serial.println(relay2);
      }
      else {
        Serial.println("Error occurred while sending HTTP POST");
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
