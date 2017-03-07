#include <Arduino.h>

#include <FS.h>
#include <ArduinoOTA.h>
#include <time.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Hash.h>
#include <WebSocketsServer.h>

#include <SSD1306.h>
#include <SSD1306Ui.h>


// Global vars
char* webImage       = (char*)malloc(1024 * sizeof(char));

// Change as you like
const char *ssid         = NULL;
const char *password     = NULL;
const char *otaPassword  = "password";

SSD1306          display    (0x3C, D5, D6);
SSD1306Ui        ui         ( &display );

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

#include "icons.h"
#include "DrawFunctions.h"
#include "LoadingFunctions.h"

void setup() {
  Serial.begin(115200);
  memset(webImage, 0, 1024 * sizeof(uint8_t));

  Serial.begin(115200);

  ui.setTargetFPS(60);

  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);

  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, FRAME_COUNT);

  ui.init();

  display.flipScreenVertically();
  display.setContrast(255);

  ui.runLoadingProcess(loadingStages, LOADING_STAGES_COUNT);
}


void loop() {
  int timeBudget = ui.update();
  unsigned long ms = millis();

  ArduinoOTA.handle();

  server.handleClient();
  webSocket.loop();

  int remainingBudget = timeBudget - (millis() - ms);

  if (remainingBudget > 0) {
    delay(remainingBudget);
  }
}
