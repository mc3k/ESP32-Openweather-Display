/**
 * ==============================================================================
 * @file    ESP32_C3_OpenWeather_OLED.ino
 * @brief   OpenWeatherMap 2.5 Display Client for SSD1306 OLED (128x64)
 * @target  ESP32-C3 Dev Module
 * @pins    SDA = GPIO 6, SCL = GPIO 7
 * 
 * @details Features custom 16x16 weather bitmaps, dynamic font scaling for the
 *          temperature unit, and automatic wind speed metric conversion (km/h).
 * 
 * @dependencies
 *  - U8g2 Library (oliver)
 *  - ArduinoJson Library (bblanchon)
 * 
 * @license MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files...
 * ==============================================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include "Icons32.inc.h"
#include "Icons16.inc.h"

// --- WiFi Credentials ---
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// --- OpenWeatherMap Configuration ---
const char* apiKey   = "YOUR_OPENWEATHER_API_KEY";
const char* city     = "London";
const char* country  = "GB";

// Free API 2.5 Current Weather Endpoint URL layout
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(country) + "&appid=" + String(apiKey) + "&units=metric";

// --- Hardware Pin Definitions (ESP32-C3) ---
#define SDA_PIN 6
#define SCL_PIN 7

// --- Driver instantiation ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

// --- OpenWeather Icon Matcher ---
const unsigned char* getBitmapByIconId(String iconId) {
  if (iconId == "01d") return sun_32x32;
  if (iconId == "01n") return clear_sky_night_32x32;
  if (iconId == "02d") return few_clouds_32x32;
  if (iconId == "02n") return few_clouds_night_32x32;
  if (iconId == "03d" || iconId == "03n") return scattered_clouds_32x32;
  if (iconId == "04d") return broken_clouds_32x32;
  if (iconId == "04n") return broken_clouds_night_32x32;
  if (iconId == "09d" || iconId == "09n") return shower_rain_32x32; 
  if (iconId == "10d" || iconId == "10n") return rain_32x32;
  if (iconId == "11d" || iconId == "11n") return thunderstorm_32x32;
  if (iconId == "13d" || iconId == "13n") return snow_32x32;
  if (iconId == "50d" || iconId == "50n") return mist_32x32;
  return default_clouds_32x32; 
}

inline const char* getHeading(float num) {
  int val = int((num / 45) + 0.5);
  static const char* arr[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  return arr[val % 8];
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  updateWeatherData();
}

void loop() {
  delay(900000); // 15-minute refresh cadence
  updateWeatherData();
}

void updateWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println(payload);
      
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      serializeJsonPretty(doc, Serial); Serial.println();

      if (!error) {
        float temp = doc["main"]["temp"];
        int humidity = doc["main"]["humidity"];
        
        // Extract raw wind speed parameter (meters per second)
        float windSpeedMps = doc["wind"]["speed"]; 
        float windDeg = doc["wind"]["deg"]; 
        // Convert metrics: m/s * 3.6 = km/h
        float windSpeedKmh = windSpeedMps * 3.6; 
        
        String weatherIcon = doc["weather"][0]["icon"].as<String>(); 
        const unsigned char* activeBitmap = getBitmapByIconId(weatherIcon);
        
        u8g2.clearBuffer();
        
        // Large Temperature Readout Line
        u8g2.setFont(u8g2_font_logisoso32_tn);
        String tempStr = String(temp, 0);
        u8g2.drawStr(15+2, 32, tempStr.c_str());
        int textWidth = u8g2.getStrWidth(tempStr.c_str());
        u8g2.setFont(u8g2_font_helvB12_tf);
        int smallUnitXPosition = 15+2 + textWidth + 2;
        u8g2.drawStr(smallUnitXPosition, 12, "\260");
        u8g2.setFont(u8g2_font_helvR08_tf);
        u8g2.drawStr(smallUnitXPosition+7, 8, "C");
       
        // Secondary Metadata Info Rows
        u8g2.setFont(u8g2_font_logisoso16_tf);
        String humStr = String(humidity);
        u8g2.drawStr(16+1, 64, humStr.c_str());
        textWidth = u8g2.getStrWidth(humStr.c_str());
        u8g2.setFont(u8g2_font_helvB08_tf);
        smallUnitXPosition = 16 +1+ textWidth + 2;
        u8g2.drawStr(smallUnitXPosition, 64-16+8, "%"); 

        // Formatted Wind Speed Text String
        u8g2.setFont(u8g2_font_logisoso16_tf);
        String windStr = String(windSpeedKmh, 0);
        textWidth = u8g2.getStrWidth(windStr.c_str());
        u8g2.drawStr(128-textWidth-21, 64, windStr.c_str());
        u8g2.setFont(u8g2_font_helvR08_tf);
        smallUnitXPosition = 58+8 + textWidth + 2;
        u8g2.drawStr(128-18, 64-16+8, "kmh"); 
        //u8g2.drawStr(smallUnitXPosition, 64-8+8, "/h"); 
        
        // Draw mercury temp line
        u8g2.drawBox(0, 64-16-8, 128, 2);

        // Wind direction
        Serial.print("Wind Direction: ");Serial.println(getHeading(windDeg));
        
        // Positioned 16x16 weather asset (Top Right Corner Area)
        u8g2.setBitmapMode(1); 
        u8g2.drawXBMP(128-32, 0, 32, 32, activeBitmap); 
        u8g2.drawXBMP(-2, 64-16, 16, 16, humidity_16x16); 
        u8g2.drawXBMP(128-textWidth-21-14, 64-15, 16, 16, wind_16x16); 
        u8g2.drawXBMP(-1, 0, 16, 32, thermometer_16x32); 
        u8g2.drawBox(6, 21-int(temp/2),1,int(temp/2));
        u8g2.sendBuffer(); 
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
