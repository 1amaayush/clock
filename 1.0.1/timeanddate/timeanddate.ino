#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include <LiquidCrystal_I2C.h>

//0x3F or 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);   //LCD Object

const char* ssid       = "GNIOT-FF-151-152";
const char* password   = "987654321";

const char* ntpServer1 = "in.pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 19800;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }

  //Display Date & Time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("%02d-%02d-%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  lcd.setCursor(0, 1);
  lcd.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void setup()
{
  Serial.begin(115200);

  // Setup LCD with backlight and initialize
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  lcd.clear();
  lcd.print("CONNECTED");
  delay(2000);

  // Set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  // Set time configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void loop()
{
  delay(1000);
  printLocalTime();
}
