
#include <WiFi.h>
// #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ArduinoWebsockets.h>
#include <DifferentialSteering.h>
#include "batteryctrl.h"
#include "motorCtrl.h"
#include "dashboard.h"
#include "wifiCtrl.h"

unsigned long currentMillis;
unsigned long previousMillis = 0;
unsigned long lastScreenUpdateMillis = 0;
int updateInterval = 500;

const char *ssid = "Wave_Rover";
const char *password = "12345678";

String message = "";

// Create AsyncWebServer object on port 80
// using namespace websockets;
// WebsocketsServer WSserver;
AsyncWebServer server(80);
// AsyncEventSource events("/events");
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// <<<<<<<<<<=== === ===SSD1306: 0x3C=== === ===>>>>>>>>>>
// 0.91inch OLED
bool screenDefaultMode = true;
String wifiSSID;
String wifiIP;
String screenLine_0;
String screenLine_1;
String screenLine_2;
String screenLine_3;

#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void InitScreen() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.display();
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  InitScreen();
  initWiFi();
  pinInit();

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  // Handle Web Server Events
  // events.onConnect([](AsyncEventSourceClient *client) {
  //   if (client->lastId()) {
  //     Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
  //   }
  //   // send event with message "hello!", id current millis
  //   // and set reconnect delay to 1 second
  //   client->send("hello!", NULL, millis(), 10000);
  //   events.send("0", "voltage", millis());
  // });

  Serial.println("Starting server");
  // server.addHandler(&events);

  server.begin();
  // WSserver.listen(82);
  Serial.println("Starting websocket");
}

void loop() {

  // auto client = WSserver.accept();
  // client.onMessage(handle_message);
  // while (client.available()) {
    currentMillis = millis();
    ws.cleanupClients();
    if (currentMillis - previousMillis > updateInterval) {
      previousMillis = currentMillis;
       InaDataUpdate();
      // events.send("ping", NULL, millis());
      // events.send(String(loadVoltage_V).c_str(), "voltage", millis());

      // getWifiStatus();
      // allDataUpdate();
      //notifyClients(getSensorReadings());
    }
    // client.poll();
  // }
}