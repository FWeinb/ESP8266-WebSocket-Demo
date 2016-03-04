// Websocket callback
void webSocketCallback(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_BIN:
            memcpy(webImage, payload, length * sizeof(uint8_t));
            break;
        case WStype_TEXT:
            char* data = (char *) payload;
            if (data[0] == 'd') {
              uint8_t color;
              int x;
              int y;

              char* command = strtok(data, ":");
              byte i = 0;
              while (command != NULL) {
                  switch (i) {
                    case 1:
                      color = atoi(command);
                      break;
                    case 2:
                      x = atoi(command);
                      break;
                    case 3:
                      y = atoi(command);
                      break;
                  }
                  command = strtok(NULL, ":");
                  i++;
              }
              webImage[x * (DISPLAY_HEIGHT/8) + y / 8] |=  (1 << (y & 7));
            } else if (data[0] == 'c') {
              memset(webImage, 0, DISPLAY_BUFFER_SIZE);
            } else if (data[0] == 'n') {
              ui.nextFrame();
            } else if (data[0] == 'p') {
              ui.previousFrame();
            } else if (data[0] == 'a') {
              ui.disableAutoTransition();
            } else if (data[0] == 'e') {
              ui.enableAutoTransition();
            }
            break;
    }

}
