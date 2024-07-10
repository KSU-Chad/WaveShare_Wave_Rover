
#include <Arduino.h>
#include "driver/gpio.h"
#include <WiFi.h>
// #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ArduinoWebsockets.h>
#include "DifferentialSteering.h"
#include "dashboard.h"


unsigned long currentMillis;
unsigned long previousMillis = 0;
unsigned long lastScreenUpdateMillis = 0;
int updateInterval = 500;

//Access-point SSID
const char *ssid = "SlowCar";
const char *password = "87654321";

//Connect to home network
// const char *ssid = "Replace_with_your_SSID";
// const char *password = "Replace_with_your_password";

String message = "";

// Create AsyncWebServer object on port 80
// using namespace websockets;
// WebsocketsServer WSserver;
AsyncWebServer server(80);
// AsyncEventSource events("/events");
AsyncWebSocket ws("/ws");

IPAddress IP;

// Json Variable to Hold Sensor Readings
JSONVar readings;

// <<<<<<<<<<=== === ===SSD1306: 0x3C=== === ===>>>>>>>>>>
// 0.91inch OLED
bool screenDefaultMode = false;
String wifiSSID;
String wifiIP;
String screenLine_0;
String screenLine_1;
String screenLine_2;
String screenLine_3;

const int PWMA = 25;  // Motor A PWM control  Orange
const int AIN2 = 17;  // Motor A input 2      Brown
const int AIN1 = 21;  // Motor A input 1      Green
const int BIN1 = 22;  // Motor B input 1       Yellow
const int BIN2 = 23;  // Motor B input 2       Purple
const int PWMB = 26;  // Motor B PWM control   White

int freq = 5000;

const int ledChannel1 = 2;
const int ledChannel2 = 4;
const int resolution = 8;

DifferentialSteering DiffSteer;
int fPivYLimit = 100;
int idle = 0;
#include <Wire.h>

#define S_SCL 33
#define S_SDA 32
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

// Initialize WiFi
void initWiFi() {
  //**for home network connection
   //WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);

  //**for stand-alone wifi
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();

  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(IP);
}
void notifyClients(String state) {
  ws.textAll(state);
}

void pinInit() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);

  ledcAttachPin(PWMA, ledChannel1);
  ledcAttachPin(PWMB, ledChannel2);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void motorDrive(int speed, int channel, int motorPin1, int motorPin2) {
  if (speed == 0) {
    Serial.println("zero");
    speed = 0;
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, HIGH);
    ledcWrite(channel, 0);
  } else if (speed > 10) {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    ledcWrite(channel, abs(speed));
    Serial.print(motorPin1);
    Serial.print(motorPin2);
    Serial.print("pin1: ");
    Serial.print(digitalRead(motorPin1), DEC);
    Serial.print(" pin2: ");
    Serial.print(digitalRead(motorPin2), DEC);
    Serial.print("forword: ");
    Serial.println(ledcRead(channel));
  } else if (speed < -10) {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    ledcWrite(channel, abs(speed));
    Serial.print(motorPin1);
    Serial.print(motorPin2);
    Serial.print("pin1: ");
    Serial.print(digitalRead(motorPin1), DEC);
    Serial.print(" pin2: ");
    Serial.print(digitalRead(motorPin2), DEC);
    Serial.print(" reverse: ");
    Serial.println(ledcRead(channel));
  } else {
    Serial.println("JK");
  }
}


void differential_steer(int XValue, int YValue) {
  // Outside no action limit joystick
  // if (!((XValue > -5) && (XValue < 5) && (YValue > -5) && (YValue < 5)||(YValue==0)))
  if (!(((XValue > -5) && (XValue < 5)) || ((YValue > -5) && (YValue < 5)))) {
    Serial.print(XValue);
    Serial.print(",");
    Serial.print(YValue);

    // XValue = map(XValue, -90, 90, -255, 255); // 0-180
    // YValue = map(YValue, -90, 90, 255, -255); // 0-180 reversed

    DiffSteer.computeMotors(XValue, YValue);
    int leftMotor = DiffSteer.computedLeftMotor();
    int rightMotor = DiffSteer.computedRightMotor();

    // sendMessage("leftMotor", String(leftMotor));
    // sendMessage("rightMotor", String(rightMotor));

    leftMotor = map(leftMotor, -127, 127, -255, 255);    // 0-180
    rightMotor = map(rightMotor, -127, 127, -255, 255);  // 0-180 reversed
    Serial.print(" ** ");
    Serial.print(leftMotor);
    Serial.print(",");
    Serial.println(rightMotor);
    motorDrive(leftMotor, ledChannel1, AIN1, AIN2);
    motorDrive(rightMotor, ledChannel2, BIN1, BIN2);
  } else {
    motorDrive(idle, ledChannel1, AIN1, AIN2);
    motorDrive(idle, ledChannel2, BIN1, BIN2);
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char *)data;
    if (message.indexOf("js") >= 0) {
      int commaIndex = message.indexOf(',');
      int steerValue = message.substring(2, commaIndex).toInt();
      int driveValue = message.substring(commaIndex + 1).toInt();
      steerValue = map(steerValue, -90, 90, -127, 127);  // 0-180
      driveValue = map(driveValue, -90, 90, 127, -127);  // 0-180 reversed
      differential_steer(steerValue, driveValue);
    }
  }
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
  Wire.begin(S_SDA, S_SCL);
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
  // while (client.available()) {All
  currentMillis = millis();
  ws.cleanupClients();
  if (currentMillis - previousMillis > updateInterval) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("SSID: "));
    display.print(ssid);
    display.setCursor(0, 8);
    display.print(F("IP: "));
    display.print(IP);
    display.display();
    previousMillis = currentMillis;
  }
}
