// Initialize WiFi
void initWiFi() {
  // WiFi.mode(WIFI_AP);
  // WiFi.begin(ssid, password);
  WiFi.softAP(ssid, password);
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}
void notifyClients(String state) {
  ws.textAll(state);
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