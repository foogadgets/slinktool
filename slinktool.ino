#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>
#include <ArduinoSpotify.h>
#include <MusicAlbumTOC.h>
#include <Sony_SLink.h>

#include "Credentials.h"
#include "webpages.h"

#define SLINK_INPIN 5                   // GPIO5 (D1)
#define SLINK_OUTPIN 4                  // GPIO4 (D2)
#define SERIAL_COM_PORT_SPEED 115200UL  // 115 kbps
#define SIZEOFTEMP 560

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClientSecure WiFiClient;
ArduinoSpotify spotify(WiFiClient);
Slink slink;
SpotifyDevice *myDevices;
static MusicAlbumTOC myTOC;

static const char *hostName = "slinktool";
static const char *callbackUri = "http%3A%2F%2Fslinktool.local%2Fcallback";
static uint8_t stateToken = 0;
static char tmpBuf[SIZEOFTEMP];  // big buffer for misc. use.
static uint8_t trackDurationIndex = 1;
static unsigned long trackMarkTime = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      {
        switch (payload[0]) {
          case '0':
            {  // PKG RECV Album name
              uint8_t tocStatus = 0;
              myTOC.clearTOC();
              tocStatus = myTOC.setAlbumName((char *)&payload[1]);
              if (tocStatus != 0) {
                Serial.print("setAlbumName failed! Error: ");
                Serial.println(tocStatus);
              } else {
                Serial.println("Album name set.");
              }
            }
            break;
          case '1':
            {  // PKG RECV Track names
              uint8_t tocStatus = 0;
              char *pch;
              char endMarker[] = "\r\n";
              pch = strtok((char *)&payload[1], endMarker);
              while (pch != NULL) {
                tocStatus = myTOC.addTrack(pch);
                if (tocStatus != 0) {
                  Serial.print("addTrack ");
                  Serial.print(myTOC.getNoTracks());
                  Serial.print(" failed! Error: ");
                  Serial.println(tocStatus);
                } else {
                  Serial.println("Track added.");
                }
                pch = strtok(NULL, endMarker);
              }
            }
            break;
          case '2':
            {  // PKG RECV Track duration
              uint8_t tocStatus = 0;
              char *pch;
              char endMarker[] = ";";
              uint8_t i = 1;
              pch = strtok((char *)&payload[1], endMarker);
              while (pch != NULL) {
                tocStatus = myTOC.setTrackDuration(i, atol(pch));
                if (tocStatus != 0) {
                  Serial.print("getNoTracks(): ");
                  Serial.println(myTOC.getNoTracks());
                  Serial.print("setTrackDuration track ");
                  Serial.print(i);
                  Serial.print(" failed! Error: ");
                  Serial.println(tocStatus);
                } else {
                  Serial.println("Track duration set.");
                }
                pch = strtok(NULL, endMarker);
                i++;
              }
              webSocket.sendTXT(0, "m:Done");
            }
            break;
          case '3':
            {  // PKG RECV Recording trigger
              Serial.println("~~~~~~~~~~~~~ Autorecording from Spotify ~~~~~~~~~~~~~");
              stateToken = 3;
            }
            break;
          case '4':
            {  // PKG RECV Write to MD
              stateToken = 4;
            }
            break;
          case '5':
            {  // PKG RECV with ClientID
              spotify.setClientId((char *)&payload[1]);
            }
            break;
          case '6':
            {  // PKG RECV with ClientSecret
              spotify.setClientSecret((char *)&payload[1]);
            }
            break;
          case '7':
            {  // PKG RECV with Token renewal check trigger
              Serial.println("~~~ Checking for token renewal ~~~");
              stateToken = 7;
            }
            break;
          case '8':
            {  // In case we are in stateToken=5 (recording) we interrupt that process.
              slink.sendCommand(SLINK_DEVICE_MD, SLINK_CMD_MD_STOP);
              Serial.println("Interrupting recording.");
              trackDurationIndex = 1;
              stateToken = 0;
              webSocket.sendTXT(0, "x:Recording interrupted.");
            }
            break;
          default:
            break;
        } // switch(payload[0])
      }
      break;
    default:
      break;
  }
}

void sendTokens() {
  memset(tmpBuf, 0, SIZEOFTEMP * sizeof(char));
  strncpy(tmpBuf, "t:", 2);
  strncat(tmpBuf, spotify.getAccessToken(), (SIZEOFTEMP - 1 - 2));
  webSocket.sendTXT(0, tmpBuf);
}

void handleLogin() {
  server.send_P(200, "text/html", login_template);
}

void handleCallback() {
  memset(tmpBuf, 0, SIZEOFTEMP * sizeof(char));
  const char *refreshToken = NULL;
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "code") {
      strncpy(tmpBuf, server.arg(i).c_str(), (SIZEOFTEMP - 1));
      refreshToken = spotify.requestAccessTokens(tmpBuf, callbackUri);
    }
  }

  if (refreshToken != NULL) {
    sendTokens();
    server.send_P(200, "text/html", login_next);
  } else {
    server.send(404, "text/plain", "Failed to load token, check serial monitor");
  }
}

void handleNotFound() {
  memset(tmpBuf, 0, sizeof(char) * SIZEOFTEMP);
  strncpy(tmpBuf, "--File Not Found ->\n", 22);
  strncat(tmpBuf, "\tURI: ", 8);
  strcat(tmpBuf, server.uri().c_str());
  strncat(tmpBuf, "\n", 3);

  for (uint8_t i = 0; i < server.args(); i++) {
    strncat(tmpBuf, " ", 2);
    strcat(tmpBuf, server.argName(i).c_str());
    strncat(tmpBuf, ": ", 3);
    strcat(tmpBuf, server.arg(i).c_str());
    strncat(tmpBuf, "\n", 2);
  }

  Serial.print(tmpBuf);
  server.send(404, "text/plain", tmpBuf);
}

void setup() {
  Serial.begin(SERIAL_COM_PORT_SPEED);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println();
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  WiFiClient.setInsecure();

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin(hostName)) {
    Serial.print("Browse to:  ");
    Serial.print("http://");
    Serial.print(hostName);
    Serial.println(".local");
  }

  slink.init(SLINK_INPIN, SLINK_OUTPIN);  // Set-up S-Link pin
  Serial.print("S-Link input pin ");
  Serial.println(slink.inPin());
  Serial.print("S-Link output pin ");
  Serial.println(slink.outPin());

  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });

  server.on("/login", handleLogin);
  server.on("/callback", handleCallback);
  server.onNotFound(handleNotFound);

  server.begin();

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loop() {

  switch (stateToken) {
    case 3:  // Auto record from Spotify
      {
        trackDurationIndex = 1;
        myDevices = spotify.scanDevices();
        Serial.println("  Spotify:\tPausing Spotify.");
        spotify.pause(myDevices->id);
        spotify.setVolume(0, myDevices->id);
        delay(2000);  // delay between each command
        Serial.println("MiniDisc:\tPrepare recording. Pressing REC & PAUSE");
        slink.sendCommand(SLINK_DEVICE_MD, SLINK_CMD_MD_REC_PAUSE);
        delay(2000);  // delay between each command

        Serial.println("  Spotify:\tPlay music .....");
        spotify.setVolume(100, myDevices->id);
        delay(150);
        webSocket.sendTXT(0, "p:Go");

        Serial.println("MiniDisc:\tStart recording");
        trackMarkTime = myTOC.getTrackDuration(trackDurationIndex++) + millis();
        delay(5); // TIME THIS DELAY //
        slink.sendCommand(SLINK_DEVICE_MD, SLINK_CMD_MD_PLAY);
        stateToken = 5;  // Start to send track marks
      }
      break;
    case 5:  // Setting track marks according to spotify track length since this is more
      {      // reliable than letting new track detection in the MD recorder do it.
        if (millis() > trackMarkTime) {
          if (trackDurationIndex > myTOC.getNoTracks()) {
            slink.sendCommand(SLINK_DEVICE_MD, SLINK_CMD_MD_STOP);
            Serial.println("Recording done!");
            trackDurationIndex = 1;
            stateToken = 0;
            webSocket.sendTXT(0, "x:All done.");
          } else {
            slink.sendCommand(SLINK_DEVICE_MD, SLINK_CMD_MD_REC_PAUSE);
            trackMarkTime = myTOC.getTrackDuration(trackDurationIndex++) + millis();
          }
        }
      }
      break;
    case 4:  // Write TOC to MD
      {
        Serial.println("-- Writing TOC to MiniDisc --");
        Serial.println("------------ Album -----------");
        // Write Disk Title
        Serial.println(myTOC.getAlbumName());
        if (!slink.writeDiskTitle(myTOC.getAlbumName())) {
          Serial.println("!! Failed to write Album name");
          break;
        }
        // Write Song Titles
        Serial.println("------------ Songs -----------");
        for (uint8_t i = 1; i <= myTOC.getNoTracks(); i++) {
          Serial.print(myTOC.getTrackName(i));
          Serial.print(": ");
          Serial.println(myTOC.getTrackDuration(i));
          if (!slink.writeTrackTitle(i, myTOC.getTrackName(i))) {
            Serial.print("!! Failed to write title number ");
            Serial.println(i);
            break;
          }
        }
        Serial.printf("------------\nNumber of tracks: %d\n\n", myTOC.getNoTracks());
        stateToken = 0;
      }
      break;
    case 7:
      {
        Serial.println("Checking if Token need refresh.");
        spotify.checkAndRefreshAccessToken();
        sendTokens();
        stateToken = 0;
      }
      break;
    default:
      break;
  }  // switch()

  webSocket.loop();
  server.handleClient();
  MDNS.update();
}
