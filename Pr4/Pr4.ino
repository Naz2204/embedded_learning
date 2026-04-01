#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "ZIROCHKA";
const char* password = "1911DIMA";

const int xPin = 34;
const int yPin = 35;
const int zPin = 13;

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

void setup() {
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(xPin, INPUT_PULLUP);
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

void readAndSend() {
  int xRaw = analogRead(xPin);
    int yRaw = analogRead(yPin);

    int xCenter = 1880;
    int yCenter = 1880;

    int xFinal = xRaw - xCenter;
    int yFinal = yRaw - yCenter;

    int xMapped, yMapped;

    if (xFinal >= 0) 
      xMapped = map(xRaw, xCenter, 4095, 0, 100);
    else 
      xMapped = map(xRaw, 0, xCenter, -100, 0);

    if (yFinal >= 0) 
      yMapped = map(yRaw, yCenter, 4095, 0, 100);
    else 
      yMapped = map(yRaw, 0, yCenter, -100, 0);

    xMapped = constrain(xMapped, -100, 100);
    yMapped = constrain(yMapped, -100, 100);
 
    String jsonResponse = "{\"x\":" + String(xMapped) + ",\"y\":" + String(yMapped) + "}";
    Serial.println(jsonResponse);
    ws.textAll(jsonResponse);
}

void loop() {
  ws.cleanupClients();

  static unsigned long lastTime = 0;

  if (millis() - lastTime > 50) { 
    readAndSend();
    
    lastTime = millis();
  }
}