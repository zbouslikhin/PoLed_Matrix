/*******************************************************************
    Basic Websocket Example

    Written by Brian Lough
    https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA

    June 5th, 2018 - Added !rect, !line & !circle - Seon Rozenblum (Unexpected maker)
 *******************************************************************/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>

// ----------------------------
// Standard Libraries - Already Installed if you have ESP8266 set up
// ----------------------------

#include <Ticker.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------


#include <PxMatrix.h>
// The library for controlling the LED Matrix
// Needs to be manually downloaded and installed
// https://github.com/2dom/PxMatrix

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

Ticker display_ticker;

// Pins for LED MATRIX
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_OE 2
#define P_D 12
#define P_E 0

// PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
// PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

//------- Replace the following! ------
const char *ssid = "CGA2121_ZKXZrVE";
const char *password = "64WFrt6JDneuLgZCKz";

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  char xArray[4];
  int x;
  int y;
  int w;
  int h;
  uint16_t colour;
  int commaCount = 0;
  String inPayload;
  String colourString;

  int commas[] = {-1,-1,-1,-1}; // using 4 for now
  int command;
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
      // Serial.printf("[%u] get Text: %s\n", num, payload);
      inPayload = String((char *) payload);
      Serial.println(inPayload);

      if (inPayload == "CLEAR") {
        clearDisplay();
      } else {

        // clear commas
        // need to makr this use size of
        for ( int i = 0; i < ELEMENTS(commas); i++ )
          commas[i] = -1;

        // grab all comma positions
        int commaIndex = 0;
        for (int i = 0; i < inPayload.length(); i++ )
        {
          if ( inPayload[i] == ',' )
            commas[ commaIndex++ ] = i;
        }
        
        /* commands
        0 = draw
        1 = rect
        2 = line
        3 = cirle
        4 = text ?????
        */

        // grab command
        int commandSeperator = inPayload.indexOf(":");
        command = inPayload.substring(0,commandSeperator).toInt();
        Serial.println(command);
        
        x = inPayload.substring(commandSeperator+1, commas[0]).toInt();
        y = inPayload.substring(commas[0] + 1, commas[1]).toInt();

        Serial.print(x);
        Serial.print(",");
        Serial.println(y);
      
        if ( command == 0 ) // draw pixel
        {
          colourString = inPayload.substring(commas[1] + 1);
          Serial.println(colourString);
          colour = strtol(colourString.c_str(), NULL, 0);
          Serial.println(colour);
          display.drawPixel(x , y, colour);
        }
        else if ( command == 1 ) // rect
        {
          w = inPayload.substring(commas[1] + 1, commas[2]).toInt();
          h = inPayload.substring(commas[2] + 1, commas[3]).toInt();
          colourString = inPayload.substring(commas[3] + 1);
          colour = strtol(colourString.c_str(), NULL, 0);

          display.drawRect( x, y, w, h, colour );

        }
        else if ( command == 2 ) // line
        {
          w = inPayload.substring(commas[1] + 1, commas[2]).toInt();
          h = inPayload.substring(commas[2] + 1, commas[3]).toInt();
          colourString = inPayload.substring(commas[3] + 1);
          colour = strtol(colourString.c_str(), NULL, 0);

          display.drawLine( x, y, w, h, colour );
        }
        else if ( command == 3 ) // circle
        {
          w = inPayload.substring(commas[1] + 1, commas[2]).toInt();
          colourString = inPayload.substring(commas[2] + 1);
          colour = strtol(colourString.c_str(), NULL, 0);

          display.drawCircle( x, y, w, colour );
        }


        


        // Serial.print("X: ");
        // Serial.println(x);
        // Serial.print("Y: ");
        // Serial.println(y);
        // Serial.print("Colour: ");
        // Serial.println(colour, HEX);


      }


      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }

}

void display_updater()
{

  display.display(70);

}

void clearDisplay(){
  for(int i = 0; i < 64; i++){
    for(int j = 0; j < 32; j++){
      display.drawPixel(i , j, 0x0000);
    }
  }
}

void startWiFi() {
  /*-----START : Network configuration-----*/
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*-----END : Network configuration-------*/
}

void setup() {

  Serial.begin(9600);

  display.begin(16);
  display.clearDisplay();

  display_ticker.attach(0.002, display_updater);
  yield();

startWiFi();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

}
