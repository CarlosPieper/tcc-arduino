#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "Torquetti";
const char* password = "dezpila2020";
const char* serverName = "http://suaapi.com/endpoint";

WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);  // Comunicação com o ATmega328P
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
}

void loop() {
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');

    if (receivedData.startsWith("OCUPADO")) {
      int firstComma = receivedData.indexOf(',');
      int secondComma = receivedData.indexOf(',', firstComma + 1);

      String lat = receivedData.substring(firstComma + 1, secondComma);
      String lng = receivedData.substring(secondComma + 1);

      sendHttpPost(lat, lng);
    }
  }
}

void sendHttpPost(String lat, String lng) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(wifiClient, "192.168.0.1", 5000, "/", false);
    http.addHeader("Content-Type", "application/json");

    String requestBody = "{\"latitude\": " + lat + ", \"longitude\": " + lng + "}";
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Resposta: " + response);
    } else {
      Serial.println("Erro ao enviar POST: " + String(httpResponseCode));
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
}
