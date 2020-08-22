/*
 - WiFi Button V2.0 -
Commander Red 22.08.2020

- Should Support any ESP8266 Devices, for ESP32 support you would have to replace / add compatible librarys.
- Tested with NodeMCU ESP8266 Dev Boards (NodeMCU Amica)

Functions:
 - uses Thinger.io for app access (will be released later)
 - uses WakeOnLan to turn on your PC via mac address (if not working, check if WakeOnLan; WoL; is enabled in your network card settings)
 - uses WiFiClient to make http-GET requests to urls (IFTTT, etc.)
 - Supports Arduino "Over The Air"-programming to easily program the micro-controller without any USB cable

 - Default mode list:
      Mode 0: WakeOnLan over MAC-address
      Mode 1-5: 
      Mode 6 is hardcoded to restart the ESP8266. If more modes are added, mode switcher will jump over mode 6. (Mode 6 is selected from thinger.io cloud)
*/

//Program Settings
#define BAUD_RATE 115200 //Define your baud rate here (Defaults are 115200 / 9600)
#define OTA_PASSWORD "admin" //Arduino OTA password for uploading wirelessly

//WiFi config
const char *ssid = "";
const char *password = "";

//Thinger.io config (https://console.thinger.io/)
#define USERNAME "" //Enter account name & device details. For reference see https://docs.thinger.io/quick-sart/quick-start#2-connect-device
#define DEVICE_ID ""
#define DEVICE_CREDENTIAL ""

//Pin config
#define r D2         //Red LED pin
#define g D3         //Green LED pin
#define b D7         //Blue LED pin
#define buttonPin D5 //Button Pin - Connect button between this Pin and GND

//Action config
const char *MACAddress = ""; //Enter mac adress for WakeOnLan
  //Here you can add http GET urls like IFTTT webhooks
const char *web1 = "";
const char *web2 = "";
const char *web3 = "";
const char *web4 = "";
const char *web5 = "";

//No more settings

#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WakeOnLan.h>
#include <ThingerESP8266.h>


#define SSID ssid
#define SSID_PASSWORD password
#ifndef STASSID
#define STASSID SSID
#define STAPSK SSID_PASSWORD
#endif

WiFiUDP UDP;
WakeOnLan WOL(UDP);
ESP8266WiFiMulti WiFiMulti;

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
bool fading = true;
bool fadingWeb = true;
bool buttonPressed = false;
bool buttonPress = false;
bool triggered;
bool longEvent = false;
bool shortEvent = false;
bool activateWeb = false;

int timerR = 0;
int timerB = 0;
int timerG = 0;
int noFade = 0;
int redPin = r;   // Red LED,   connected to digital pin 9
int greenPin = g; // Green LED, connected to digital pin 10
int bluePin = b;  // Blue LED,  connected to digital pin 11
int redVal = 255; // Variables to store the values to send to the pins
int greenVal = 1; // Initial values are Red full, Green and Blue off
int blueVal = 1;
int i = 0;     // Loop counter
int wait = 10; // 50ms (.05 second) delay; shorten for faster fades
int buttonMode = 0;
int modeCount = 6;
int delayLongPress = 300;
int PressType = 3;
int function = 255;
unsigned long timer1, timer2;
unsigned long pressTime;

void nextMode()
{
  if (buttonMode < modeCount - 1)
  {
  buttonMode++;
  if (buttonMode == 6)
    {
      buttonMode++;
    }
}
else
{
  buttonMode = 0;
}
Serial.print("buttonMode: ");
Serial.println(buttonMode);
}

void loop()
{
  ArduinoOTA.handle();
  WiFiClient client;
  HTTPClient http;
  thing.handle();
  ledHandle();
  buttonHandle();
  if (function < 255)
  {
    switch (function)
    {
    case 0:
      //Mode 0 - WoL
      timerR = 2000;
      digitalWrite(g, LOW);
      digitalWrite(b, LOW);
      WOL.sendMagicPacket(MACAddress);
      break;
    case 1:
      //Mode 1
      timerG = 2000;
      digitalWrite(r, LOW);
      digitalWrite(b, LOW);
      http.begin(client, web1);
      http.GET();
      http.end();
      break;
    case 2:
      //Mode 2
      timerB = 2000;
      digitalWrite(g, LOW);
      digitalWrite(r, LOW);
      http.begin(client, web2);
      http.GET();
      http.end();
      break;
    case 3:
      //Mode 3
      timerR = 2000;
      timerG = 2000;
      digitalWrite(b, LOW);
      http.begin(client, web3);
      for (int j = 0; j < 5; j++)
      {
        http.GET();
        http.end();
        delay(10);
      }
      http.end();
      break;
    case 4:
      //Mode 4
      timerR = 2000;
      timerB = 2000;
      digitalWrite(g, LOW);
      http.begin(client, web4);
      for (int j = 0; j < 5; j++)
      {
        http.GET();
        http.end();
        delay(10);
      }
      http.end();
      break;
    case 5:
      //Mode 5
      timerB = 2000;
      timerG = 2000;
      digitalWrite(r, LOW);
      http.begin(client, web5);
      http.GET();
      http.end();
      break;
    case 6:
      ESP.restart();
      break;
    }
    function = 255;
  }
  fading = fadingWeb;
  if (fading == false)
  {
    noFade = 2000;
  }
}

void buttonHandle()
{
  buttonPressed = !(digitalRead(buttonPin));
  if (buttonPressed && buttonPress == false)
  {
    pressTime = millis();
    buttonPress = true;
    //One Time executed if button pressed
    Serial.println("Button pressed");
  }
  else if (!buttonPressed)
  {
    buttonPress = false;
    //Executed in loop after button releases / if not pressed
    longEvent = false;
    shortEvent = false;
    PressType = 3;
  }

  if (buttonPressed)
  {
    noFade = 2200;
    while (PressType == 3)
    {
      Serial.println("In while");
      whatPress();
      switch (PressType)
      {
      case 0:
        //Not pressed
        Serial.println("no press");
        break;
      case 1:
        //Long press
        Serial.println("Long press");
        timerR = 2000;
        timerG = 2000;
        timerB = 2000;
        nextMode();
        break;
      case 2:
        //Short press
        Serial.println("Short press");
        function = buttonMode;
        break;
      }
    }
  }
}

void ledHandle()
{
  if (noFade > 0)
  {
    noFade--;
  }
  else
  {
    fadingF();
  }
  if (timerR > 0)
  {
    timerR--;
    analogWrite(r, timerR);
  }
  else
  {
    digitalWrite(r, LOW);
  }
  if (timerG > 0)
  {
    timerG--;
    analogWrite(g, timerG);
  }
  else
  {
    digitalWrite(g, LOW);
  }
  if (timerB > 0)
  {
    timerB--;
    analogWrite(b, timerB);
  }
  else
  {
    digitalWrite(b, LOW);
  }
  delay(1);
}

void fadingF()
{
  i += 1;      // Increment counter
  if (i < 255) // First phase of fades
  {
    redVal -= 1;   // Red down
    greenVal += 1; // Green up
    blueVal = 1;   // Blue low
  }
  else if (i < 509) // Second phase of fades
  {
    redVal = 1;    // Red low
    greenVal -= 1; // Green down
    blueVal += 1;  // Blue up
  }
  else if (i < 763) // Third phase of fades
  {
    redVal += 1;  // Red up
    greenVal = 1; // Green low
    blueVal -= 1; // Blue down
  }
  else // Re-set the counter, and start the fades again
  {
    i = 1;
  }
  analogWrite(redPin, redVal); // Write current values to LED pins
  analogWrite(greenPin, greenVal);
  analogWrite(bluePin, blueVal);
  delay(wait);
}

boolean nodelay(unsigned long &since, unsigned long time)
{
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time)
  {
    since = currentmillis;
    return true;
  }
  return false;
}

void whatPress()
{
  delay(delayLongPress);
  if (digitalRead(buttonPin) == false)
  {
    PressType = 1;
  }
  else
  {
    PressType = 2;
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(r, HIGH);
  digitalWrite(g, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(16, HIGH);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  thing.add_wifi(SSID, SSID_PASSWORD);
  WiFi.begin(ssid, password);
  thing["buttonMode"] << [](pson &in) {
    buttonMode = in;
  };

  thing["fading"] << [](pson &in) {
    fading = in;
    fadingWeb = in;
  };

  thing["wait"] << [](pson &in) {
    wait = in;
  };

  thing["toggleFade"] << [](pson &in) {
    fadingWeb = !fadingWeb;
  };
  thing["restart"] << [](pson &in) {
    bool restarting = in;
    if (restarting)
    {
      ESP.restart();
      restarting = false;
    }
  };

  thing["modeCount"] << [](pson &in) {
    modeCount = in;
  };
  digitalWrite(r, LOW);
  digitalWrite(g, LOW);
  digitalWrite(b, LOW);
}