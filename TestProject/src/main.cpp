#include <Arduino.h>
#include <ESP8266WiFi.h>

#define BUFFER_SIZE 100
#define WIFI_SSID "WIFI NAME"
#define WIFI_PASSWORD "WIFI PASSWORD"

char receivedMessage[BUFFER_SIZE];
int bufferIndex = 0;

void initWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  // initWifi();
  Serial.println("ESP8266 is ready.\n");
}

void loop()
{
  while (Serial.available())
  {
    char c = Serial.read();

    if (c == '\n' || c == '\r')
    {
      if (bufferIndex > 0)
      {
        receivedMessage[bufferIndex] = '\0';

        Serial.print("Received: ");
        Serial.println(receivedMessage);

        bufferIndex = 0;
      }
    }
    else if (bufferIndex < BUFFER_SIZE - 1)
    {
      receivedMessage[bufferIndex++] = c;
    }
  }
}