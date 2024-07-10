// Initialize WiFi
void initWiFi();
void notifyClients(String state);

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);