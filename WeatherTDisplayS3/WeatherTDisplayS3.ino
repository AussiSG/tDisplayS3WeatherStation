#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager  version 2.0.17
#include <TFT_eSPI.h>
#include <ArduinoJson.h> // 7.1.0
#include <HTTPClient.h> // https://github.com/arduino-libraries/ArduinoHttpClient   version 0.6.1
#include <ESP32Time.h>  // https://github.com/fbiego/ESP32Time  verison 2.0.6
#include <pins_arduino.h>

#include "NotoSansBold15.h"
#include "tinyFont.h"
#include "smallFont.h"
#include "midleFont.h"
#include "bigFont.h"
#include "font18.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite errSprite = TFT_eSprite(&tft);
ESP32Time rtc(0);


//#################### EDIT THIS  ###################
//time zone  
int zone = 2;
String town = "YourWeatherLocation"; // desired location must be available on https://openweathermap.org/ 
String myAPI = "YourOpenWeatherAPI";
String units = "metric";  //  metric, imperial
//#################### end of edits ###################


const char* ntpServer = "pool.ntp.org";
String server = "https://api.openweathermap.org/data/2.5/weather?q=" + town + "&appid=" + myAPI + "&units=" + units;


//additional variables
int ani = 100;
float maxT;
float minT;
unsigned long timePased = 0;
int counter=0;

//................colors
#define bck TFT_BLACK
unsigned short grays[13];

//screen + backlight
const int pwmFreq = 10000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
const int backlight[5] = {10, 30, 60, 120, 220};

// static strings of data showed on right side 
char* PPlbl[] = { "HUM", "PRESS", "WIND" };
String PPlblU[] = { " %", " hPa", " m/s" };

//data that changes
float temperature = 22.2;
float wData[3];
float PPpower[24] = {};    //graph
float PPpowerT[24] = {};   //graph
int PPgraph[24] = { 0 };   //graph
int press1 = 0;
int press2 = 0;
byte curBright = 1;

//scroling message on bottom right side
String Wmsg = "";

void setTime() {
  configTime(3600 * zone, 0, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  }
}

void setup() {

  // physical buttons
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  // using this board can work on battery
  pinMode(15,OUTPUT);
  digitalWrite(15,1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Connecting to WIFI!!",30,50,4);
  sprite.createSprite(320, 170);
  errSprite.createSprite(164, 15);


  ////set brightness
  #if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(TFT_BL, pwmLedChannelTFT);
    ledcWrite(pwmLedChannelTFT, backlight[curBright]);
  #else
    ledcAttach(TFT_BL, pwmFreq, pwmResolution);
    ledcWrite(TFT_BL, backlight[curBright]);
  #endif

  //connect board to wifi , if cant, esp32 will make wifi network, connect to that network with password "password"
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(5000);

  if (!wifiManager.autoConnect("S3WeatherStation", "password")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }
  Serial.println("Connected.");
  setTime();
  getData();

  // generate 13 levels of gray
  int co = 210;
  for (int i = 0; i < 13; i++) {
    grays[i] = tft.color565(co, co, co);
    co = co - 20;
  }
}

void getData() {
  HTTPClient http;
  http.begin(server);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("server response");
    Serial.println(payload);

    // Parsiranje JSON odgovora
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      temperature = doc["main"]["temp"];
      wData[0] = doc["main"]["humidity"];
      wData[1] = doc["main"]["pressure"];
      wData[2] = doc["wind"]["speed"];

      int visibility = doc["visibility"];
      const char* description = doc["weather"][0]["description"];
      long dt = doc["dt"];

      Wmsg = "#Description: " + String(description) + "  #Visbility: " + String(visibility) + " #Updated: " + rtc.getTime();

      // Show temperature on serial monitor
      Serial.print("Temperature: ");
      Serial.print(temperature);

    } else {
      Serial.print("ERROR JSON-a: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP ERROR ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


void draw() {

  errSprite.fillSprite(grays[10]);
  errSprite.setTextColor(grays[1], grays[10]);
  errSprite.drawString(Wmsg, ani, 4);

  sprite.fillSprite(TFT_BLACK);
  sprite.drawLine(138, 10, 138, 164, grays[6]);
  sprite.drawLine(100, 108, 134, 108, grays[6]);
  sprite.setTextDatum(0);

  //LEFTSIDE
  sprite.loadFont(midleFont);
  sprite.setTextColor(grays[1], TFT_BLACK);
  sprite.drawString("WEATHER", 6, 10);
  sprite.unloadFont();

  sprite.loadFont(font18);
  sprite.setTextColor(grays[7], TFT_BLACK);
  sprite.drawString("TOWN:", 6, 110);
  sprite.setTextColor(grays[2], TFT_BLACK);
  sprite.fillCircle(118, 52, 2, grays[2]);
  if (units == "metric")
    sprite.drawString("C", 124, 50);
  if (units == "imperial")
    sprite.drawString("F", 124, 50);

 
  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.drawString(town, 46, 110);
  sprite.unloadFont();

  // draw time without seconds
  sprite.loadFont(tinyFont);
  sprite.setTextColor(grays[0], TFT_BLACK);
  sprite.drawString(rtc.getTime().substring(0, 5), 6, 132);
  sprite.unloadFont();

  // draw some static text
  sprite.setTextColor(grays[5], TFT_BLACK);
  sprite.drawString("INTERNET", 86, 10);
  sprite.drawString("STATION", 86, 20);
  sprite.setTextColor(grays[7], TFT_BLACK);
  sprite.drawString("SECONDS", 92, 157);

  // draw temperature
  sprite.setTextDatum(4);
  sprite.loadFont(bigFont);
  sprite.setTextColor(grays[0], TFT_BLACK);
  sprite.drawFloat(temperature, 1, 69, 80);
  sprite.unloadFont();


  //draw sec rectangle
  sprite.fillRoundRect(90, 132, 42, 22, 2, grays[2]);
  //draw seconds
  sprite.loadFont(font18);
  sprite.setTextColor(TFT_BLACK, grays[2]);
  sprite.drawString(rtc.getTime().substring(6, 8), 111, 144);
  sprite.unloadFont();


  sprite.setTextDatum(0);
  //RIGHT SIDE
  sprite.loadFont(font18);
  sprite.setTextColor(grays[1], TFT_BLACK);
  sprite.drawString("LAST 12 HOUR", 144, 10);
  sprite.unloadFont();

  sprite.fillRect(144, 28, 84, 2, grays[10]);

  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.drawString("MIN:" + String(minT), 254, 10);
  sprite.drawString("MAX:" + String(maxT), 254, 20);
  sprite.fillSmoothRoundRect(144, 34, 174, 60, 3, grays[10], bck);
  sprite.drawLine(170, 39, 170, 88, TFT_WHITE);
  sprite.drawLine(170, 88, 314, 88, TFT_WHITE);

  sprite.setTextDatum(4);

  for (int j = 0; j < 24; j++)
    for (int i = 0; i < PPgraph[j]; i++)
      sprite.fillRect(173 + (j * 6), 83 - (i * 4), 4, 3, grays[2]);

  sprite.setTextColor(grays[2], grays[10]);
  sprite.drawString("MAX", 158, 42);
  sprite.drawString("MIN", 158, 86);

  sprite.loadFont(font18);
  sprite.setTextColor(grays[7], grays[10]);
  sprite.fillCircle(152, 52, 2, grays[7]);
  if (units == "metric")
    sprite.drawString("C", 160, 58);
  if (units == "imperial")
    sprite.drawString("F", 160, 58);
  sprite.unloadFont();


  for (int i = 0; i < 3; i++) {
    sprite.fillSmoothRoundRect(144 + (i * 60), 100, 54, 32, 3, grays[9], bck);
    sprite.setTextColor(grays[3], grays[9]);
    sprite.drawString(PPlbl[i], 144 + (i * 60) + 27, 107);
    sprite.setTextColor(grays[2], grays[9]);
    sprite.loadFont(font18);
    sprite.drawString(String((int)wData[i])+PPlblU[i], 144 + (i * 60) + 27, 124);
    sprite.unloadFont();

    sprite.fillSmoothRoundRect(144, 148, 174, 16, 2, grays[10], bck);
    errSprite.pushToSprite(&sprite, 148, 150);
  }
  sprite.setTextColor(grays[4], bck);
  sprite.drawString("CURR. WEATHER", 190, 141);

  sprite.setTextColor(grays[4], bck);
  sprite.drawString("BRIGHT:", 260, 141);
 // sprite.setTextColor(grays[9], bck);
 //  sprite.drawString(String(counter), 310, 141);
  
  for(int i = 0; i <= curBright; i++)
    sprite.fillRect(280 + (i * 7), 135, 3, 10, TFT_WHITE);
  
  sprite.pushSprite(0, 0);
}

void updateData() {

  //update alsways
  //part needed for scroling weather msg
  ani--;
  if (ani < -420)
    ani = 100;


  if (digitalRead(BUTTON_2) == 0) {
      if (press2 < 5)
          press2++;
      else if (press2 == 5) {  // turn off back light of TFT
          press2++;
  #if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
              ledcWrite(pwmLedChannelTFT, 0);
  #else
              ledcWrite(TFT_BL, 0);
  #endif
          }
      }
    else if (press2 > 0) {
        if (press2 < 5) {
            sprite.fillRect(280, 135, 44, 12, TFT_BLACK);
            if(++curBright == 5) curBright = 0;
            for(int i = 0; i <= curBright; i++)
               sprite.fillRect(280 + (i * 7), 135, 3, 10, TFT_WHITE);
  #if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
              ledcWrite(pwmLedChannelTFT, backlight[curBright]);
  #else
              ledcWrite(TFT_BL, backlight[curBright]);
  #endif
          }
          press2 = 0;
      }

  if (millis() > timePased + 180000) {
    timePased = millis();
    counter++;
    getData();

    if (counter==10) {
       setTime();
       counter=0;
       maxT = -50;
       minT = 1000;
      PPpower[23] = temperature;
      for (int i = 23; i > 0; i--)
        PPpower[i - 1] = PPpowerT[i];

      for (int i = 0; i < 24; i++) {
        PPpowerT[i] = PPpower[i];
        if (PPpower[i] < minT) minT = PPpower[i];
        if (PPpower[i] > maxT) maxT = PPpower[i];
      }

      for (int i = 0; i < 24; i++) {
        PPgraph[i] = map(PPpower[i], minT, maxT, 0, 12);
      }
    }
  }
}

void loop() {

  updateData();
  draw();
}