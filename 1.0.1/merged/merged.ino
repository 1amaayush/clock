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

//WiFi ssid and password.
const char* ssid = "GNIOT-FF-151-152";
const char* password = "987654321";

//ntp server config.
const char* ntpServer1 = "in.pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

unsigned long previousMillis = 0;
const long interval = 5000; // interval for switching display (2 seconds).
bool showTime = true; // flag to switch between time and attendance.

float atnd;
float cond;
float data;

int index1;
int index2;
int ip;

void printLocalTime() { //function to print time in the final loop
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }

  // Display Date & Time.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(&timeinfo, "%I:%M:%S %p :)"); //if this seems confusing, refer to the link below.
  lcd.setCursor(0, 1);
  lcd.print(&timeinfo, "%a %d %b %Y");
  //specifiers info- "https://en.cppreference.com/w/cpp/chrono/c/strftime". //edit the format using this link.
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void fetchAndDisplayAttendance() {
  HTTPClient http;
  String url = "https://gnioterp.com:155/StudentAcademicReportct.aspx?StuId=72534&SessionId=1022&ClassId=73&SemId=142"; //attendance data url from gnioterp.
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      // int index = payload.indexOf("GridView1_ctl13_lblT3StTotal"); //cell id of total attendance percentage.
      index1 = payload.indexOf("GridView1_ctl13_lblT1StTotal"); //getting the data off the server.
      index2 = payload.indexOf("GridView1_ctl13_lblT2StTotal");

      if (index1 != -1) {
        // String data = payload.substring(index + 33, index + 35); //just in case total attendence is to be displayed directly.
        String data1 = payload.substring(index1 + 33, index1 + 36); //total class conducted span id.
        String data2 = payload.substring(index2 + 33, index2 + 36); //total class attended span id.
        Serial.println(data1); 
        Serial.println(data2);
        cond = data1.toFloat(); //converting string to float.
        atnd = data2.toFloat(); //float(s).
        data = (atnd/cond)*100; //calculating percentage by simply dividing number of classes attended by total classes conducted.
        Serial.println(data);
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

  // setup LCD with backlight and initialize.
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  //logo?
  lcd.clear();
  lcd.print(".- -.-- ..- ... ");
  delay(1000);


  // connect to WiFi
  lcd.clear();
  Serial.printf("Connecting to %s ", ssid);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  delay(500);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ip = WiFi.localIP();
  Serial.println(ip);
  Serial.println("connected");
  lcd.clear();
  lcd.print("connected to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi");
  delay(800);

  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  // set time configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    showTime = !showTime; //toggle the flag.

    if (showTime) {
      printLocalTime(); //prints time and date info.
    } else {
      fetchAndDisplayAttendance(); //prints attendance info.
    }
  }
}
//.- -.-- ..- ...