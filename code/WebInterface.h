#pragma once
#include <ESPAsyncWebServer.h>

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void setupWebServer();

void loadCalibration();

void broadcastSync();
