#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <Wire.h>
#include <TimeLib.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
//Ping
#include <ESP8266Ping.h>
//DHT
#include <DHT.h>
DHT dht(5, DHT11);  //DHT sensor pin GPIO 5 (D1)

/*  Desk Clock System
 *  https://ahmetozer.org
 *  Do not forget weather settings and add all lines from font.h to libraries/ESP8266_and_ESP32_Oled_Driver_for_SSD1306_display/src OLEDDisplayFonts.h
 *  DHT sensor connected at D1(gpio5) pin on nodemcu
 *  OLED screen connected at D2 and D3 pins
*/

String      weatherapikey   = "0000000000000000000000000000";   //  openweathermap.org api key
String      weatherlocation = "304782";                             //  Your city location code. Example Marmaris/Turkey 304782 https://openweathermap.org/city/304782
String      weatherunit     = "metric";                             //  metric or imperial
const char* ssid            = "ahmetozer.org";                     //  Wifi name
const char* password        = "ahmetozer.org";                       //  Wifi password
int         gmtsec          = 10800;                                //  add seconds to utc time . 3600 second per hour.

//Varibles
int wapi = 0; // weater check
int dd=0;     //weather now check
float hum;
float cel;
float hic;


// Time SYNC
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.cloudflare.com", gmtsec, 60000);
int t;
// OLED
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// Include the UI lib
#include "OLEDDisplayUi.h"
// Include custom images
#include "images.h"


SSD1306Wire  display(0x3c, D3, D2);       // pin settings
OLEDDisplayUi ui ( &display );
int screenW = 128;
int screenH = 64;
int clockCenterX = screenW/2;
int clockCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int clockRadius = 23;

// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  ui.disableIndicator();
// Draw the clock face
  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  String datenow = String(day())+"/"+String(month())+"/"+String(year());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , 10 - y, timenow );
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y , datenow );

  dd=0; // Reset
  wapi = 0;
}


void weathernow(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (dd == 0){
    hum = dht.readHumidity();
    cel = dht.readTemperature();
    hic = dht.computeHeatIndex(hum, cel, false);
    if (isnan(hum) || isnan(cel)) { // Check if any reads failed and exit early (to try again).
     return;
    }
    dd = 1;
  }

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , 10 + y, String(cel)+"C°" );              //  Temperature
  display->drawString(clockCenterX + x , 26 + y , "Hum "+String(hum) );           //  Humidity
  display->drawString(clockCenterX + x , clockCenterY + y , "Dew "+String(hic) ); //  Dew point
}

int aa = 0; // Check every 100 times
// Ping results
int ms1;
int ms2;
int ms3;
int ms4;
int ms5;
int ms6;
int ms7;
int ms8;

/*
 * Server Ping Adress
 */

void pings(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if(aa == 0) {
    Ping.ping("8.8.8.8", 1);
    ms1 = Ping.averageTime();
    Ping.ping("google.com", 1);
    ms2 = Ping.averageTime();
    Ping.ping("cloudflare.com", 1);
    ms3 = Ping.averageTime();
    Ping.ping("216.66.84.46", 1);
    ms4 = Ping.averageTime();
    Ping.ping("89.107.228.9", 1);
    ms5 = Ping.averageTime();
    Ping.ping("ahmetozer.org", 1);
    ms6 = Ping.averageTime();
    Ping.ping("10.0.0.5", 1);
    ms7 = Ping.averageTime();
    Ping.ping("81.212.205.149", 1);
    ms8 = Ping.averageTime();
  }
  aa = aa + 1;
  if (aa == 100) {
    aa = 0;
  }
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(clockCenterX + x , 10 , "8.8.8.8 "+String(ms1)+"  google "+String(ms2) );
  display->drawString(clockCenterX + x , 20 , "he.nl " + String(ms4)+"  Cloudflare "+String(ms3) );
  display->drawString(clockCenterX + x , 30 , "DGN " + String(ms5)+"  ahmetozer.org"+String(ms6) );
  display->drawString(clockCenterX + x , 40 , "Router " + String(ms7)+"  Izmir "+String(ms8) );
}

// This array keeps function pointers to all frames
// frames are the single views that slide in


FrameCallback frames[] = { digitalClockFrame, netusage, weathernow, weatherapi, pings};                 // Frames

// how many frames are there?
int frameCount = 5;
// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;




void setup() {
  Serial.begin(9600);
  timeClient.begin();

  ui.setTargetFPS(60);
  display.init();
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  ui.setIndicatorPosition(TOP);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  display.flipScreenVertically();

  //  Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //  Waiting for network connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  //  NTP Time update
  timeClient.update();
  setTime(timeClient.getEpochTime());

  //  OTA Update
  ArduinoOTA.setHostname("Desk-Clock");
  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  // DHT START
  dht.begin(); // initialize dht
    // Oled Screen Brightness
    Wire.beginTransmission(0x3c);
    Wire.write(0x00);
    Wire.write(0x81);
    Wire.endTransmission();
    Wire.beginTransmission(0x3c);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();
}


void loop() {
  ArduinoOTA.handle();

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}

String weatherapin;

void weatherapi(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
StaticJsonBuffer<1200> jsonBuffer;
if (wapi == 0){
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://api.openweathermap.org/data/2.5/weather?id="+weatherlocation+"&appid="+weatherapikey+"&units="+weatherunit);  //Specify request destination
    int httpCode = http.GET();
    weatherapin = http.getString();
    http.end();   //Close connection
    wapi = 1;
    }
        //Serial.println(weatherapin);
        JsonObject& owm_data = jsonBuffer.parseObject(weatherapin);
        if (!owm_data.success()) {
          //Serial.println("Parsing failed");
          return;
        }
        String temp = owm_data["main"]["temp"];
        String temp_min = owm_data["main"]["temp_min"];
        String temp_max = owm_data["main"]["temp_max"];
        String temp_des = owm_data["weather"][0]["main"];

        String humidity = owm_data["main"]["humidity"];
        String pressure = owm_data["main"]["pressure"];

        String wname = "Sun";

        String wind_s =   owm_data["wind"]["speed"];
        String wind_deg = owm_data["wind"]["deg"];
        int sunrise  = owm_data["sys"]["sunrise"];
        int sunset   = owm_data["sys"]["sunset"];
        sunrise = sunrise + gmtsec;
        sunset = sunset + gmtsec;


  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(clockCenterX + x , 10 + y , temp+"C°"+" Min "+ temp_min+" Max "+temp_max  );
  display->drawString(clockCenterX + x , 21 + y , " W "+ wind_s +" "+wind_deg +"    "+temp_des );
  display->drawString(clockCenterX + x , 32 + y , wname +"rise️ "+hour(sunrise)+":"+minute(sunrise)+" Sunset "+hour(sunset)+":"+minute(sunset) );
  display->drawString(clockCenterX + x , 43 + y , "Hum "+humidity+" P "+pressure);
  }

int nettime = 0;

void netusage(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
StaticJsonBuffer<400> jsonBuffer;
    if (nettime == 0){
      HTTPClient http;  //Declare an object of class HTTPClient
      http.begin("http://10.0.0.5:777/desk-clock.php");  //Do not forget this settings. İf you dont use a this function you can remove on line 190 netusage and line 193 int frameCount = 4;
      int httpCode = http.GET();
      weatherapin = http.getString();
      http.end();   //Close connection
    }
    nettime = nettime + 1;
    if (nettime == 50) {
      nettime = 0;
    }
     //Serial.println(weatherapin);
     JsonObject& owm_data = jsonBuffer.parseObject(weatherapin);
     if (!owm_data.success()) {
     //Serial.println("Parsing failed");
     return;
     }
        String ppp0rx = owm_data["ppp0"]["down"];
        String ppp0tx = owm_data["ppp0"]["up"];
        String ppp0rxs = owm_data["ppp0"]["rxs"];
        String ppp0txs = owm_data["ppp0"]["txs"];

        String sit1rx = owm_data["sit1"]["down"];
        String sit1tx = owm_data["sit1"]["up"];
        String sit1rxs = owm_data["sit1"]["rxs"];
        String sit1txs = owm_data["sit1"]["txs"];


  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x , 10 + y , "TT "+ppp0rxs+" "+ppp0txs);
  display->drawString(x , 20 + y , ppp0rx+" "+ppp0tx);
  display->drawString(x , 30 + y , "HE "+sit1rxs+" "+sit1txs);
  display->drawString(x , 40 + y , sit1rx+" "+sit1tx);

}
