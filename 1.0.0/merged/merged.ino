//program to display time, date then attendance.
#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// LCD object at address 0x27 with a 16x2 screen.
LiquidCrystal_I2C lcd(0x27, 16, 2);

//WiFi ssid and password
const char* ssid = "GNIOT-FF-151-152";
const char* password = "987654321";

//ntp server config
const char* ntpServer1 = "in.pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

unsigned long previousMillis = 0;
const long interval = 5000; // Interval for switching display (2 seconds)
bool showTime = true; // Flag to switch between time and attendance

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }

  // Display Date & Time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(&timeinfo, "%b %d %Y");
  lcd.setCursor(0, 1);
  lcd.print(&timeinfo, "%A,%I:%M:%S %p");
  //specifiers info- "https://en.cppreference.com/w/cpp/chrono/c/strftime"
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void fetchAndDisplayAttendance() {
  HTTPClient http;
  String url = "https://gnioterp.com:155/StudentAcademicReportct.aspx?StuId=72534&SessionId=1022&ClassId=73&SemId=142"; //Attendane data url from gnioterp.
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      int index = payload.indexOf("GridView1_ctl13_lblT3StTotal"); //cell id of total attendance percentage.
      if (index != -1) {
        String data = payload.substring(index + 33, index + 35); // Adjust substring based on data format
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Attendance:");
        lcd.setCursor(0, 1);
        lcd.print(data);
        lcd.print(" %");
      }
    }
  } else {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void setup() {
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

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    showTime = !showTime; // Toggle the flag

    if (showTime) {
      printLocalTime(); //prints time and date info
    } else {
      fetchAndDisplayAttendance(); //prints attendance info
    }
  }
}
