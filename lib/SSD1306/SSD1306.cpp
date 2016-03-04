/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
 */

#include "SSD1306.h"


SSD1306::SSD1306(uint8_t i2cAddress, uint8_t sda, uint8_t sdc) {
  this->i2cAddress = i2cAddress;
  this->sda = sda;
  this->sdc = sdc;
}

bool SSD1306::init() {
  this->buffer = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  if(!this->buffer) {
    DEBUG_SSD1306("[SSD1306][init] Not enough memory to create display\n");
    return false;
  }

  #ifdef SSD1306_DOUBLE_BUFFER
  this->buffer_back = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  if(!this->buffer_back) {
    DEBUG_SSD1306("[SSD1306][init] Not enough memory to create back buffer\n");
    free(this->buffer);
    return false;
  }
  #endif

  Wire.begin(this->sda, this->sdc);

  // Let's use ~700khz if ESP8266 is in 160Mhz mode
  // this will be limited to ~400khz if the ESP8266 in 80Mhz mode.
  Wire.setClock(700000);

  sendInitCommands();

  resetDisplay();

  return true;
}

void SSD1306::end() {
  if (this->buffer) free(this->buffer);
  #ifdef SSD1306_DOUBLE_BUFFER
  if (this->buffer_back) free(this->buffer_back);
  #endif
}

void SSD1306::resetDisplay(void) {
  clear();
  #ifdef SSD1306_DOUBLE_BUFFER
  memset(buffer_back, 1, DISPLAY_BUFFER_SIZE);
  #endif
  display();
}

void SSD1306::reconnect() {
  Wire.begin(this->sda, this->sdc);
}

void SSD1306::setColor(SSD1306_COLOR color) {
  this->color = color;
}

void SSD1306::setPixel(int16_t x, int16_t y) {
  if (x >= 0 && x < 128 && y >= 0 && y < 64) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * DISPLAY_WIDTH] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y / 8) * DISPLAY_WIDTH] ^=  (1 << (y & 7)); break;
    }
  }
}

void SSD1306::drawRect(int16_t x, int16_t y, int16_t width, int16_t height) {
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width, y, height);
  drawHorizontalLine(x, y + height, width);
}

void SSD1306::fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height) {
  for (int16_t i = yMove; i < yMove + height; i++) {
    drawHorizontalLine(xMove, i, width);
  }
}

void SSD1306::drawHorizontalLine(int16_t x, int16_t y, int16_t length) {
  if (y < 0 || y >= DISPLAY_HEIGHT) { return; }

  if (x < 0) {
    length += x;
    x = 0;
  }

  if ( (x + length) > DISPLAY_WIDTH) {
    length = (DISPLAY_WIDTH - x);
  }

  if (length <= 0) { return; }

  uint8_t * bufferPtr = buffer;
  bufferPtr += (y >> 3) * DISPLAY_WIDTH;
  bufferPtr += x;

  uint8_t drawBit = 1 << (y & 7);

  switch (color) {
    case WHITE:   while (length--) {
        *bufferPtr++ |= drawBit;
      }; break;
    case BLACK:   drawBit = ~drawBit;   while (length--) {
        *bufferPtr++ &= drawBit;
      }; break;
    case INVERSE: while (length--) {
        *bufferPtr++ ^= drawBit;
      }; break;
  }
}

void SSD1306::drawVerticalLine(int16_t x, int16_t y, int16_t length) {
  if (y < 0 || y > DISPLAY_HEIGHT) return;

  if (x < 0) {
    length += x;
    x = 0;
  }

  if (length < 0) return;


  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = buffer;

  bufferPtr += (y >> 3) * DISPLAY_WIDTH;
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (color) {
      case WHITE:   *bufferPtr |= drawBit; break;
      case BLACK:   *bufferPtr &= drawBit; break;
      case INVERSE: *bufferPtr ^= drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += DISPLAY_WIDTH;
  }

  if (length >= 8) {
    switch (color) {
      case WHITE:
      case BLACK:
        drawBit = (color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += DISPLAY_WIDTH;
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += DISPLAY_WIDTH;
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << length & 7) - 1;
    switch (color) {
      case WHITE:   *bufferPtr |= drawBit; break;
      case BLACK:   *bufferPtr &= drawBit; break;
      case INVERSE: *bufferPtr ^= drawBit; break;
    }
  }
}

void SSD1306::drawFastImage(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *image) {
  drawInternal(xMove, yMove, width, height, image, 0, 0);
}

void SSD1306::drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *xbm) {
  int16_t widthInXbm = (width + 7) / 8;
  uint8_t data;

  for(int16_t y = 0; y < height; y++) {
    for(int16_t x = 0; x < width; x++ ) {
      if (x & 7) {
        data >>= 1; // Move a bit
      } else {  // Read new data every 8 bit
        data = pgm_read_byte(xbm + (x / 8) + y * widthInXbm);
      }
      // if there is a bit draw it
      if (data & 0x01) {
        setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void SSD1306::drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth) {
  uint8_t textHeight       = pgm_read_byte(fontData + HEIGHT_POS);
  uint8_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t sizeOfJumpTable = pgm_read_byte(fontData + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  uint8_t cursorX         = 0;
  uint8_t cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      yMove -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      xMove -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      xMove -= textWidth;
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth  < 0 || xMove > DISPLAY_WIDTH ) {return;}
  if (yMove + textHeight < 0 || yMove > DISPLAY_HEIGHT) {return;}

  for (uint16_t j = 0; j < textLength; j++) {
    int16_t xPos = xMove + cursorX;
    int16_t yPos = yMove + cursorY;

    byte code = text[j];
    if (code >= firstChar) {
      byte charCode = code - firstChar;

      // 4 Bytes per char code
      byte msbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      byte lsbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      byte charByteSize     = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      byte currentCharWidth = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (msbJumpToChar != 255 && lsbJumpToChar != 255) {
        // Get the position of the char data
        uint16_t charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
      }

      cursorX += currentCharWidth;
    }
  }
}


void SSD1306::drawString(int16_t xMove, int16_t yMove, String strUser) {
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  // char* text must be freed!
  char* text = utf8ascii(strUser);

  uint16_t xOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) {
    uint16_t lb;
    // Find number of linebreaks in text
    for (uint16_t i=0, lb=0; text[i]; i++) {
      lb += (text[i] == '\n');
    }
    // Calculate center
    xOffset = (lb * lineHeight) / 2;
  }

  uint16_t line = 0;
  char* textPart = strtok(text,"\n");
  while (textPart != NULL) {
    uint16_t length = strlen(textPart);
    drawStringInternal(xMove - xOffset, yMove + (line++) * lineHeight, textPart, length, getStringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }
  free(text);
}

void SSD1306::drawStringMaxWidth(int16_t xMove, int16_t yMove, uint16_t maxLineWidth, String strUser) {
  uint16_t firstChar  = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  char* text = utf8ascii(strUser);

  uint16_t length = strlen(text);
  uint16_t lastDrawnPos = 0;
  uint16_t lineNumber = 0;
  uint16_t strWidth = 0;

  uint16_t preferredBreakpoint = 0;
  uint16_t widthAtBreakpoint = 0;

  for (uint16_t i = 0; i < length; i++) {
    strWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[i] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (text[i] == ' ' || text[i]== '-') {
      preferredBreakpoint = i;
      widthAtBreakpoint = strWidth;
    }

    if (strWidth >= maxLineWidth) {
      preferredBreakpoint = preferredBreakpoint ? preferredBreakpoint : i;
      widthAtBreakpoint = preferredBreakpoint ? widthAtBreakpoint : strWidth;

      drawStringInternal(xMove, yMove + (lineNumber++) * lineHeight , &text[lastDrawnPos], preferredBreakpoint - lastDrawnPos, widthAtBreakpoint);
      lastDrawnPos = preferredBreakpoint + 1; strWidth = 0; preferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (lastDrawnPos < length) {
    drawStringInternal(xMove, yMove + lineNumber * lineHeight , &text[lastDrawnPos], length - lastDrawnPos, getStringWidth(&text[lastDrawnPos], length - lastDrawnPos));
  }

  free(text);
}

uint16_t SSD1306::getStringWidth(const char* text, uint16_t length) {
  uint16_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);

  uint16_t stringWidth = 0;
  uint16_t maxWidth = 0;

  while (length--) {
    stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10) {
      maxWidth = max(maxWidth, stringWidth);
      stringWidth = 0;
    }
  }

  return max(maxWidth, stringWidth);
}

void SSD1306::setTextAlignment(SSD1306_TEXT_ALIGNMENT textAlignment) {
  this->textAlignment = textAlignment;
}

void SSD1306::setFont(const char *fontData) {
  this->fontData = fontData;
}

void SSD1306::displayOn(void) {
  sendCommand(DISPLAYON);
}

void SSD1306::displayOff(void) {
  sendCommand(DISPLAYOFF);
}

void SSD1306::invertDisplay(void) {
  sendCommand(INVERTDISPLAY);
}

void SSD1306::normalDisplay(void) {
  sendCommand(NORMALDISPLAY);
}

void SSD1306::setContrast(char contrast) {
  sendCommand(SETCONTRAST);
  sendCommand(contrast);
}

void SSD1306::flipScreenVertically() {
  sendCommand(SEGREMAP | 0x01);      //Rotate screen 180 deg
  sendCommand(COMSCANDEC);           //Rotate screen 180 Deg
}

void SSD1306::display(void) {
  #ifdef SSD1306_DOUBLE_BUFFER
  uint16_t minBoundY = 8;
  uint16_t maxBoundY = 0;

  uint16_t minBoundX = 129;
  uint16_t maxBoundX = 0;

  uint16_t x, y;

  // Calculate the Y bounding box of changes
  // and copy buffer[pos] to buffer_back[pos];
  for (y = 0; y < 8; y++) {
     for (x = 0; x < DISPLAY_WIDTH; x++) {
      uint16_t pos = x + y * DISPLAY_WIDTH;
      if (buffer[pos] != buffer_back[pos]) {
        minBoundY = min(minBoundY, y);
        maxBoundY = max(maxBoundY, y);
        minBoundX = min(minBoundX, x);
        maxBoundX = max(maxBoundX, x);
      }
      buffer_back[pos] = buffer[pos];
    }
    yield();
  }

  // If the minBoundY wasn't updated
  // we can savely assume that buffer_back[pos] == buffer[pos]
  // holdes true for all values of pos
  if (minBoundY == 8) return;

  sendCommand(COLUMNADDR);
  sendCommand(minBoundX);
  sendCommand(maxBoundX);

  sendCommand(PAGEADDR);
  sendCommand(minBoundY);
  sendCommand(maxBoundY);

  byte k = 0;
  for (y = minBoundY; y <= maxBoundY; y++) {
      for (x = minBoundX; x <= maxBoundX; x++) {
          if (k == 0) {
            Wire.beginTransmission(this->i2cAddress);
            Wire.write(0x40);
          }
          Wire.write(buffer[x + y * DISPLAY_WIDTH]);
          k++;
          if (k == 16)  {
            Wire.endTransmission();
            k = 0;
          }
      }
      yield();
  }

  if (k != 0) {
    Wire.endTransmission();
  }

  #else
  // No double buffering
  sendCommand(COLUMNADDR);
  sendCommand(0x0);
  sendCommand(0x7F);

  sendCommand(PAGEADDR);
  sendCommand(0x0);
  sendCommand(0x7);

  for (uint16_t i=0; i < DISPLAY_BUFFER_SIZE; i++) {
    Wire.beginTransmission(this->i2cAddress);
    Wire.write(0x40);
    for (uint8_t x = 0; x < 16; x++) {
      Wire.write(buffer[i]);
      i++;
    }
    i--;
    Wire.endTransmission();
  }
  #endif
}


void SSD1306::clear(void) {
  memset(buffer, 0, DISPLAY_BUFFER_SIZE);
}


// Private functions

void SSD1306::sendCommand(unsigned char com) {
  Wire.beginTransmission(this->i2cAddress);  //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

void SSD1306::sendInitCommands(void) {
  sendCommand(DISPLAYOFF);
  sendCommand(SETDISPLAYCLOCKDIV);
  sendCommand(0xF0); // Increase speed of the display max ~96Hz
  sendCommand(SETMULTIPLEX);
  sendCommand(0x3F);
  sendCommand(SETDISPLAYOFFSET);
  sendCommand(0x00);
  sendCommand(SETSTARTLINE);
  sendCommand(CHARGEPUMP);
  sendCommand(0x14);
  sendCommand(MEMORYMODE);
  sendCommand(0x00);
  sendCommand(SEGREMAP);
  sendCommand(COMSCANINC);
  sendCommand(SETCOMPINS);
  sendCommand(0x12);
  sendCommand(SETCONTRAST);
  sendCommand(0xCF);
  sendCommand(SETPRECHARGE);
  sendCommand(0xF1);
  sendCommand(DISPLAYALLON_RESUME);
  sendCommand(NORMALDISPLAY);
  sendCommand(0x2e);            // stop scroll
  sendCommand(DISPLAYON);
}

void SSD1306::drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *data, uint16_t offset, uint16_t bytesInData) {
  if (width < 0 || height < 0) return;
  if (yMove + height < 0 || yMove > DISPLAY_HEIGHT)  return;
  if (xMove + width  < 0 || xMove > DISPLAY_WIDTH)   return;

  uint8_t  rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  int16_t initYMove   = yMove;
  int8_t  initYOffset = yOffset;


  for (uint16_t i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      yMove   = initYMove;
      yOffset = initYOffset;
    }

    byte currentByte = pgm_read_byte(data + offset + i);

    int16_t xPos = xMove + (i / rasterHeight);
    int16_t yPos = ((yMove >> 3) + (i % rasterHeight)) * DISPLAY_WIDTH;

    int16_t yScreenPos = yMove + yOffset;
    int16_t dataPos    = xPos  + yPos;

    if (dataPos >=  0  && dataPos < DISPLAY_BUFFER_SIZE &&
        xPos    >=  0  && xPos    < DISPLAY_WIDTH ) {

      if (yOffset >= 0) {
        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte << yOffset; break;
          case BLACK:   buffer[dataPos] &= currentByte << yOffset; break;
          case INVERSE: buffer[dataPos] ^= currentByte << yOffset; break;
        }
        if (dataPos < (DISPLAY_BUFFER_SIZE - DISPLAY_WIDTH)) {
          switch (this->color) {
            case WHITE:   buffer[dataPos + DISPLAY_WIDTH] |= currentByte >> (8 - yOffset); break;
            case BLACK:   buffer[dataPos + DISPLAY_WIDTH] &= currentByte >> (8 - yOffset); break;
            case INVERSE: buffer[dataPos + DISPLAY_WIDTH] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   buffer[dataPos] &= currentByte >> yOffset; break;
          case INVERSE: buffer[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }

      yield();
    }
  }
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
uint8_t SSD1306::utf8ascii(byte ascii) {
  static uint8_t LASTCHAR;

  if ( ascii < 128 ) { // Standard ASCII-set 0..0x7F handling
    LASTCHAR = 0;
    return ascii;
  }

  uint8_t last = LASTCHAR;   // get last char
  LASTCHAR = ascii;

  switch (last) {    // conversion depnding on first UTF8-character
    case 0xC2: return  (ascii);  break;
    case 0xC3: return  (ascii | 0xC0);  break;
    case 0x82: if (ascii == 0xAC) return (0x80);    // special case Euro-symbol
  }

  return  0; // otherwise: return zero, if character has to be ignored
}

// You need to free the char!
char* SSD1306::utf8ascii(String str) {
  uint16_t k = 0;
  uint16_t length = str.length() + 1;

  // Copy the string into a char array
  char* s = (char*) malloc(length * sizeof(char));
  if(!s) {
    DEBUG_SSD1306("[SSD1306][utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n");
    return (char*) str.c_str();
  }
  str.toCharArray(s, length);

  length--;

  for (uint16_t i=0; i < length; i++) {
    char c = utf8ascii(s[i]);
    if (c!=0) {
      s[k++]=c;
    }
  }

  s[k]=0;

  // This will leak 's' be sure to free it in the calling function.
  return s;
}
