#include "WebSocket.h"
#include "SPIFFSAccess.h"

LoadingStage loadingStages[] = {
  {
    .process = "Connect to WiFi",
    .callback = []() {
      WiFi.mode(WIFI_STA);

      if (ssid == NULL) {
        WiFi.begin();
      } else {
        WiFi.begin ( ssid, password );
      }
      int mil = millis();
      // Wait for connection
      while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 10 );
        Serial.print ( "." );
      }
      Serial.print ( millis() - mil );
    }
  },
  {
    .process = "Update clock",
    .callback = []() {
        configTime(0, 0, "time.nist.gov");
        while (!time(nullptr)) {
          delay(10);
        }
    }
  },
	{
		.process = "Start Web(-Socket) server",
		.callback = []() {
			SPIFFS.begin();

		  MDNS.addService("http", "tcp", 80);
		  MDNS.addService("ws", "tcp", 81);

			if (!MDNS.begin("display")) {
		    Serial.println("Error setting up MDNS responder!");
				return;
		  }

			server.onNotFound([](){
				if(!handleFileRead(server.uri())) {
					server.send(404, "text/plain", "FileNotFound");
				}
			});

			server.begin();

			webSocket.onEvent(webSocketCallback);
		  webSocket.begin();
		}
	},
  {
    .process = "Start Auto-Update server",
    .callback = [](){
        ArduinoOTA.setPassword(otaPassword);

        ArduinoOTA.onStart([]() {
          display.clear();
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.drawString(64, 20, "OTA Update");
        });

        ArduinoOTA.onEnd([]() {
          display.clear();
          display.setFont(ArialMT_Plain_16);
          display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
          display.drawString(64, 32, "Restart");
          display.display();
        });

        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
          display.drawRect(4, 32, 120, 8);
          display.fillRect(4 + 2, 32 + 2, (120 * ((float)progress / total) - 3), 8 - 3);
          display.display();
        });

        ArduinoOTA.begin();
    }
  }
};

int LOADING_STAGES_COUNT = sizeof(loadingStages) / sizeof(LoadingStage);
