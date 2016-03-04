String getFormattedTime(int timeOffset){
  time_t now = time(nullptr) + timeOffset;
  struct tm* timeinfo = localtime(&now);

  int hours = timeinfo->tm_hour;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  int minutes = timeinfo->tm_min;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  int seconds = timeinfo->tm_sec;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

void drawClock(SSD1306 *display, int16_t x, int16_t y, int timeOffset, String city, const char* icon) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 60, y + 5, city);
  display->drawXbm(x, y, 60, 60, icon);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x + 60, y + 24, getFormattedTime(timeOffset));
}

void webFrame(SSD1306 *display, SSD1306UiState* state, int16_t x, int16_t y){
  ui.disableIndicator();
  display->drawFastImage(x, y, 128, 64, webImage);
}

void berlinFrame(SSD1306 *display, SSD1306UiState* state, int16_t x, int16_t y){
  drawClock(display, x, y, 3600, "Berlin", berlin_bits);
}

void parisFrame(SSD1306 *display, SSD1306UiState* state, int16_t x, int16_t y){
  drawClock(display, x, y, 3600, "Paris", berlin_bits);
}

void newYorkFrame(SSD1306 *display, SSD1306UiState* state, int16_t x, int16_t y){
  drawClock(display, x, y, -3600 * 5, "New York", new_york_bits);
}

void londonFrame(SSD1306 *display, SSD1306UiState* state, int16_t x, int16_t y) {
  drawClock(display, x, y, 0, "London",  london_bits);
}


FrameCallback frames[] = {berlinFrame, webFrame, newYorkFrame, londonFrame };
int FRAME_COUNT = sizeof(frames) / sizeof(FrameCallback);
