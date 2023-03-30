#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Replace with your network credentials
const char* ssid = "Josh_M_iPhone";
const char* password = "12345678";

// Create a Servo object
Servo servo;

// Server details
WebServer server(80);

// Slots
const int numSlots = 6;
int slotAngles[] = {0, 60, 120, 180, 240, 300};

void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP32!");
}

void handleSlotSelect() {
  String slotId = server.arg("id");
  int slotIndex = slotId.toInt() - 1;
  if (slotIndex < 0 || slotIndex >= numSlots) {
    server.send(400, "text/plain", "Invalid slot ID");
    return;
  }
  int angle = slotAngles[slotIndex];
  servo.write(angle);
  server.send(200, "text/plain", "Slot selected: " + slotId);
}

void handleSchedule() {
  String slotId = server.arg("slot");
  int slotIndex = slotId.toInt() - 1;
  if (slotIndex < 0 || slotIndex >= numSlots) {
    server.send(400, "text/plain", "Invalid slot ID");
    return;
  }

  String timeStr = server.arg("time");
  int hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(3).toInt();
  DateTime now = rtc.now();
  DateTime scheduledTime = DateTime(now.year(), now.month(), now.day(), hour, minute);

  if (scheduledTime < now) {
    TimeSpan myTimeSpan = TimeSpan(1, 2, 30, 0); // Creates a TimeSpan object for 1 day, 2 hours, and 30 minutes

  }

  String message = "Slot " + slotId + " scheduled for " + timeStr;

  server.send(200, "text/plain", message);

  while (now < scheduledTime) {
    now = rtc.now();
    delay(1000);
  }

  int angle = slotAngles[slotIndex];
  servo.write(angle);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Attach the servo to pin 2
  servo.attach(2);

  // Move the servo to the initial position
  servo.write(0);

  // Route to handle root path
  server.on("/", HTTP_GET, handleRoot);

  // Route to handle slot selection
  server.on("/api/slot", HTTP_POST, handleSlotSelect);

  // Route to handle scheduling
  server.on("/api/schedule", HTTP_POST, handleSchedule);

  // Start server
  server.begin();

  Wire.begin();
  rtc.begin();
}

void loop() {
  server.handleClient();
}