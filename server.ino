#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <esp_now.h>

// ==== CONFIG ====
const char* ssid = "Khairul's iPhone";
const char* password = "kwebbelkop";

const int controlPin = 16;

// ==== SERVER OBJECTS ====
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Latest sensor values
float latestTemp = 0.0;
float latestHum  = 0.0;

// ==== ESP-NOW MESSAGE STRUCT ====
typedef struct struct_message {
  float temp;
  float hum;
} struct_message;

struct_message incomingData;

// ==== ESP-NOW CALLBACK ====
void onDataRecv(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  latestTemp = incomingData.temp;
  latestHum = incomingData.hum;

  Serial.printf("Received: Temp=%.1fC Hum=%.1f%%\n", latestTemp, latestHum);

  String data = "TEMP:" + String(latestTemp,1) + "C HUM:" + String(latestHum,1) + "%";
  webSocket.broadcastTXT(data);
}

// ==== WEBSOCKET EVENTS ====
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);
    Serial.printf("Client[%u] says: %s\n", num, msg.c_str());

    if (msg == "ON") {
      digitalWrite(controlPin, HIGH);
      webSocket.broadcastTXT("Pin16:ON");
    } else if (msg == "OFF") {
      digitalWrite(controlPin, LOW);
      webSocket.broadcastTXT("Pin16:OFF");
    }
  }
}

// ==== SETUP ====
void setup() {
  Serial.begin(115200);

  pinMode(controlPin, OUTPUT);
  digitalWrite(controlPin, LOW);

  if(!SPIFFS.begin(true)){
    Serial.println("Error mounting SPIFFS");
    return;
  }

  WiFi.mode(WIFI_AP_STA);  // allows ESP-NOW + WiFi at the same time
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Server IP: "); Serial.println(WiFi.localIP());

  // ESP-NOW init
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);

  // Serve HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

// ==== LOOP ====
void loop() {
  webSocket.loop();
}