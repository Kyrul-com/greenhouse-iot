#include <WiFi.h>
#include <esp_now.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22   // or DHT11
DHT dht(DHTPIN, DHTTYPE);

// Replace with MAC address of Server ESP (check with Serial.print(WiFi.macAddress()))
uint8_t serverAddress[] = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56};

// Struct to send
typedef struct struct_message {
  float temp;
  float hum;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, serverAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  dht.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    myData.temp = t;
    myData.hum = h;

    esp_err_t result = esp_now_send(serverAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.printf("Sent: Temp=%.1fC Hum=%.1f%%\n", t, h);
    } else {
      Serial.println("Error sending data");
    }
  }

  delay(2000); // send every 2 seconds
}