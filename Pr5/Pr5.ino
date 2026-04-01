#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define soundPin 34

const char* ssid = "ZIROCHKA";
const char* password = "1911DIMA";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Клієнт підключився");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("Клієнт відключився");
  }
}

void readAndSend() {
  int minVal = 4095;
  int maxVal = 0;

  for (int i = 0; i < 300; i++) {
    int val = analogRead(soundPin);
    if (val < minVal) minVal = val;
    if (val > maxVal) maxVal = val;
  }

  int amplitude = maxVal - minVal;

  String jsonResponse = String(amplitude);
  Serial.println(jsonResponse);
  ws.textAll(jsonResponse);
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення...");
  }
  Serial.println(WiFi.localIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "Готово. Відкрийте консоль браузера.");
  });

  server.begin();
}

void loop() {
  static unsigned long lastTime = 0;

  if (millis() - lastTime > 100) { 
    readAndSend();
    lastTime = millis();  
  }
}
