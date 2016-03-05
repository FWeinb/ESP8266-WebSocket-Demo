# Play A Video on an ESP8266 via WebSocket

[![Thumbnail](https://i.vimeocdn.com/video/550541560.jpg?mw=512)](https://vimeo.com/150929166) is a video showing this in action.

## How to compile

(Only tested on OS X)

  1. You need `nodejs`, `platformio` and `webpack` as well as `make`.
  2. Clone this repository
  3. Run `npm install`
  4. Connect your ESP8266 to your computer and change the `upload_port` in `platformio.ini`
  5. Add your SSID and password to `src/main.cpp`
  6. run `make uploadAll`

Connect SCL to Pin D6 and SDA to Pin D5 on a NodeMCU


## Used libs etc.
[WebSocket library](https://github.com/Links2004/arduinoWebSockets) by Links2004  
[Arduino Core ESP8266](https://github.com/esp8266/Arduino)
