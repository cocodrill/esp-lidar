#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Arduino.h>

#define LIDARSerial Serial1


char str[100];
const char *cacca = "esp-lidar";
const char *msg_cane = "esp-lidar";

const char*  dev ;
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[100];

typedef enum {
  STATE_WAIT_HEADER = 0,
  STATE_READ_HEADER,
  STATE_READ_PAYLOAD,
  STATE_READ_DONE
} State_t;

typedef struct {
  uint8_t header0;
  uint8_t header1;
  uint8_t header2;
  uint8_t header3;
  uint16_t rotation_speed;
  uint16_t angle_begin;
  uint16_t distance_0;
  uint8_t reserved_0;
  uint16_t distance_1;
  uint8_t reserved_1;
  uint16_t distance_2;
  uint8_t reserved_2;
  uint16_t distance_3;
  uint8_t reserved_3;
  uint16_t distance_4;
  uint8_t reserved_4;
  uint16_t distance_5;
  uint8_t reserved_5;
  uint16_t distance_6;
  uint8_t reserved_6;
  uint16_t distance_7;
  uint8_t reserved_7;
  uint16_t distance_8;
  uint8_t reserved_8;
  uint16_t distance_9;
  uint8_t reserved_9;
  uint16_t distance_10;
  uint8_t reserved_10;
  uint16_t distance_11;
  uint8_t reserved_11;
  uint16_t distance_12;
  uint8_t reserved_12;
  uint16_t distance_13;
  uint8_t reserved_13;
  uint16_t distance_14;
  uint8_t reserved_14;
  uint16_t distance_15;
  uint8_t reserved_15;
  uint16_t angle_end;
  uint16_t crc;
} __attribute__((packed)) LidarPacket_t;

const uint8_t header[] = { 0x55, 0xaa, 0x23, 0x10 };

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  for (uint32_t i = 0; i < len; i++) {
    src++;
  }
}

void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
      }
      break;
    case WStype_TEXT:
      {
 
      }
      break;
    case WStype_BIN:
      hexdump(payload, length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/index.html", "text/html");
}

void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/style.css", "text/css");
}

void onSCRIPTRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/SCRIPT.js", "text/javascript");
}

void onOrbitRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/OrbitControls.js", "text/javascript");
}

void onthreeRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/three.min.js", "text/javascript");
}

void oncaneRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/cane.js", "text/javascript");
}

void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(404, "text/plain", "Not found");
}

void setup(void) {

  if ( !SPIFFS.begin()) {
    while (1);
  }
  
 LIDARSerial.begin(230400, SERIAL_8N1, 3, 4);
  delay(10);

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP("esp-lidar", "12345678");

  server.on("/", HTTP_GET, onIndexRequest);
  server.on("/style.css", HTTP_GET, onCSSRequest);
  server.on("/SCRIPT.js", HTTP_GET, onSCRIPTRequest);
  server.on("/OrbitControls.js", HTTP_GET, onOrbitRequest);
  server.on("/three.min.js", HTTP_GET, onthreeRequest);
  server.on("/cane.js", HTTP_GET, oncaneRequest);
  server.onNotFound(onPageNotFound);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);


}

inline uint16_t convertDegree(uint16_t input) {
  return (input - 40960) / 64;
}

inline void remapDegrees(uint16_t minAngle, uint16_t maxAngle, uint16_t *map) {
  int16_t delta = maxAngle - minAngle;
  if (maxAngle < minAngle) {
    delta += 360;
  }
  for (int32_t cnt = 0; cnt < 16; cnt++) {
    map[cnt] = minAngle + (delta * cnt / 15);
    if (map[cnt] >= 360) {
      map[cnt] -= 360;
    }
  }
}

inline void plotDistanceMap(uint16_t* map, uint16_t* distances) {
  DynamicJsonDocument doc(2048); // Adjust size if needed
  JsonArray mapArray = doc.createNestedArray("map");
  JsonArray distancesArray = doc.createNestedArray("distances");
  for (int i = 0; i < 16; i++) {

    if (distances[i] >= 10000 ) {
      distances[i] = 0;
      mapArray.add(map[i]);
      distancesArray.add(distances[i]);
    } else {
      mapArray.add(map[i]);
      distancesArray.add(distances[i]);
    }
  }

  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.sendTXT(0, jsonString);
}

void handleLidarPacket(const uint8_t* payload) {
  const LidarPacket_t* packet = (const LidarPacket_t*)payload;
  uint16_t degree_begin = convertDegree(packet->angle_begin);
  uint16_t degree_end = convertDegree(packet->angle_end);

  if (degree_begin < 360 && degree_end < 360) {
    uint16_t map[16];
    uint16_t distances[16] = {
      packet->distance_0 & 0x3FFF, packet->distance_1 & 0x3FFF, packet->distance_2 & 0x3FFF, packet->distance_3 & 0x3FFF,
      packet->distance_4 & 0x3FFF, packet->distance_5 & 0x3FFF, packet->distance_6 & 0x3FFF, packet->distance_7 & 0x3FFF,
      packet->distance_8 & 0x3FFF, packet->distance_9 & 0x3FFF, packet->distance_10 & 0x3FFF, packet->distance_11 & 0x3FFF,
      packet->distance_12 & 0x3FFF, packet->distance_13 & 0x3FFF, packet->distance_14 & 0x3FFF, packet->distance_15 & 0x3FFF
    };
    remapDegrees(degree_begin, degree_end, map);
    plotDistanceMap(map, distances);
  }
}

void loop() {


  webSocket.loop();

  static State_t state = STATE_WAIT_HEADER;
  static uint32_t counter = 0;
  static uint8_t payload[sizeof(LidarPacket_t)];
  static uint32_t lastDataTime = 0;

  if (millis() - lastDataTime > 1000) {
    state = STATE_WAIT_HEADER;
    counter = 0;
    LIDARSerial.flush();
  }

  while (LIDARSerial.available()) {
    uint8_t data = LIDARSerial.read();
    lastDataTime = millis();

    switch (state) {
      case STATE_WAIT_HEADER:
        if (data == header[0]) {
          counter = 1;
          payload[0] = data;
          state = STATE_READ_HEADER;
        }
        break;

      case STATE_READ_HEADER:
        if (data == header[counter]) {
          payload[counter++] = data;
          if (counter == sizeof(header)) {
            state = STATE_READ_PAYLOAD;
          }
        } else {
          state = STATE_WAIT_HEADER;
          counter = 0;
        }
        break;

      case STATE_READ_PAYLOAD:
        payload[counter++] = data;
        if (counter == sizeof(LidarPacket_t)) {
          state = STATE_READ_DONE;
        }
        break;

      case STATE_READ_DONE:
        handleLidarPacket(payload);
        state = STATE_WAIT_HEADER;
        counter = 0;
        break;
    }
  }

}
