#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // for parsing JSON, if needed

const char* ssid = "GNIOT-FF-151-152";
const char* password = "987654321";

unsigned long previousMillis = 0;
const long interval = 1000; // Interval in milliseconds (30 minutes)

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initial fetch of webpage
  fetchWebpage();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // It's time to refresh
    previousMillis = currentMillis;

    fetchWebpage();
  }

  // Other loop tasks can be added here if needed
}

void fetchWebpage() {
  // Fetch webpage content
  HTTPClient http;
  String url = "https://gnioterp.com:155/StudentAcademicReportct.aspx?StuId=72534&SessionId=1022&ClassId=73&SemId=142";
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      // Parse HTML or extract the required data
      // Example parsing (you may need to adjust this based on the actual HTML structure)
      int index = payload.indexOf("GridView1_ctl13_lblT3StTotal");
      if (index != -1) {
        // Assuming the content is near the label, extract the data
        String data = payload.substring(index + 33, index + 35); // Adjust substring based on your data format
        Serial.print("Data from webpage: ");
        Serial.println(data);
      }
    }
  } else {
    Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
