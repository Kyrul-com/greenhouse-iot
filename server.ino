#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// ==== CONFIG ====
const char* ssid = "Khairul's iPhone";
const char* password = "kwebbelkop";

const int controlPin = 16;

// ==== SERVER OBJECTS ====
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// ==== WEBSOCKET EVENTS ====
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
switch(type) {
case WStype_DISCONNECTED:
Serial.printf("[%u] Disconnected!\n", num);
break;

case WStype_CONNECTED:  
  Serial.printf("[%u] Connected!\n", num);  
  break;  

case WStype_TEXT:  
  String msg = String((char*)payload);  
  Serial.printf("[%u] Message: %s\n", num, msg.c_str());  

  if (msg == "ON") {  
    digitalWrite(controlPin, HIGH);  
    webSocket.broadcastTXT("Pin16:ON");  
  } else if (msg == "OFF") {  
    digitalWrite(controlPin, LOW);  
    webSocket.broadcastTXT("Pin16:OFF");  
  }  
  break;

}
}

// ==== SETUP ====
void setup() {
Serial.begin(115200);

pinMode(controlPin, OUTPUT);
digitalWrite(controlPin, LOW);

if(!SPIFFS.begin(true)){
Serial.println("An Error has occurred while mounting SPIFFS");
return;
}

WiFi.begin(ssid, password);
Serial.print("Connecting to WiFi");
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println();
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());

// Serve HTML file
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

