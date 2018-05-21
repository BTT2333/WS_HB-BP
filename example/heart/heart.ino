#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DS1302.h>
#include <Esp.h>
#include <IOTPWebSocketsClient.h>
#include <string.h>
#include <stdlib.h>

#define SSID    "PRO6"
#define PWD     "btt123btt"
#define HEART_BEAT "HEART_BEAT"
#define RST         0
#define DAT         2
#define CLK         15


DS1302 rtc(RST, DAT, CLK);
IOTPWebSocketsClient webSocket("b41b53ea325d49e7b970fc56238bce8a", 45643124);

int co = 0;

unsigned char check, HBP, LBP, HB;
unsigned char Re_buf[11],counter;
unsigned char r[6] = {0xFD, 0x00, 0x00, 0x00, 0x00, 0x00};

void getPMInfo()
{
  unsigned char i = 0;
  Serial.write(r, 6);
  while (Serial.available()) {
    Re_buf[counter] = (unsigned char)Serial.read();
    counter++;
    if (counter == 6)             //接收到数据
    {
      counter = 0;               //重新赋值，准备下一帧数据的接收
    }
  }
  HBP=Re_buf[1];
  LBP=Re_buf[2];
  HB=Re_buf[3];
}


char * getTimeInfo()
{
  char * output = "2017-11-11 11:11:11";
  return output;
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      {
        Serial.printf("[FAIL]%s: Disconnected to server!\n", getTimeInfo());
        if (WiFi.status() != WL_CONNECTED)
        {
          Serial.printf("[failed]%s:Disconnect to WiFi\n", getTimeInfo());
        }
        Serial.printf("Reconnecting");
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.print(".");
        }
      }
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[OK]%s: Connected to url: %s\n", getTimeInfo(), payload);

        // send message to server when Connected
        webSocket.sendTXT("IOTPWebSocketClient test!", getTimeInfo());
      }
      break;
    case WStype_TEXT:
      {
        if (strcmp((char *)payload, HEART_BEAT) == 0)
        {

          char pminfo[] = "{\\\"HBP\\\": 11,\\\"LBP\\\": 11,\\\"HB\\\": 11}";

          getPMInfo();
          Serial.println(HBP);
          Serial.println(LBP);
          Serial.println(HB);
          sprintf(pminfo, "{\\\"HBP\\\": %d,\\\"LBP\\\": %d,\\\"HB\\\": %d}", HBP, LBP , HB);

          // send message to server
          webSocket.sendTXT(pminfo, getTimeInfo());
        }
        else
        {
          Serial.printf("[Message]%s: get message: %s\n", getTimeInfo(), payload);
        }
      }
      break;
    default:
      {
        Serial.printf("Default!\n");
      }
      break;
  }

}

void setup()
{
  //    webSocket.setDebugMode(9600);
  Serial.begin(115200);

  rtc.halt(false);
  rtc.writeProtect(false);

  Serial.println(getTimeInfo());

  Serial.print("Attempting to connect to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PWD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to wifi");

  webSocket.begin();

  webSocket.onEvent(webSocketEvent);

  webSocket.setReconnectInterval(2);
}

void loop()
{
  webSocket.loop();
}
