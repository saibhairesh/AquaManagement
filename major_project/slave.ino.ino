#include <ESP32Servo.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <time.h> // Include for time-related functions

Servo myServo;
const int servopin = 18;
const int trig = 25;
const int echo = 26;
volatile int inpulseCount = 0;
const int inflowSensorPin = 14;
volatile int outpulseCount = 0;
const int outflowSensorPin = 27;

#define WIFI_SSID "Rama Aditya"
#define WIFI_PASSWORD "123456789"
#define API_KEY "AIzaSyAc_vIL0GF_v3AnZsumUxW1nfsFb3Dg9qk"
#define DATABASE_URL "https://projectgriet-d4959-default-rtdb.firebaseio.com/"
#define USER_EMAIL "projectgriet25@gmail.com"
#define USER_PASSWORD "projectgriet"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

float inflowRate = 0.0;
float intotalLiters = 0.0;
unsigned long inoldTime = 0;

float outflowRate = 0.0;
float outtotalLiters = 0.0;
unsigned long outoldTime = 0;

const float calibrationFactor = 4.5;

void IRAM_ATTR inpulseCounter() {
  inpulseCount++;
}

void IRAM_ATTR outpulseCounter() {
  outpulseCount++;
}

void setup() {
  myServo.attach(servopin);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(inflowSensorPin, INPUT_PULLUP);
  pinMode(outflowSensorPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(inflowSensorPin), inpulseCounter, FALLING);
  attachInterrupt(digitalPinToInterrupt(outflowSensorPin), outpulseCounter, FALLING);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

  // Configure NTP (Network Time Protocol) for real-time clock
  configTime(0, 0, "pool.ntp.org"); // UTC time
  Serial.println("Waiting for NTP time...");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nTime synced!");
}

void loop() {
  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
    return;
  }

  // Reset inflow/outflow totals at 11:45 PM
  if (timeinfo.tm_hour == 23 && timeinfo.tm_min == 45) {
    intotalLiters = 0.0;
    outtotalLiters = 0.0;
    Serial.println("Inflow and outflow totals reset to zero.");
    delay(60000); // Wait 1 minute to avoid resetting multiple times during 11:45
  }

  // Distance using ultrasonic sensor
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  int dist = (pulseIn(echo, HIGH) * 0.034) / 2;
  Serial.println(dist);

  // Inflow sensor
  unsigned long incurrentTime = millis();
  unsigned long inelapsedTime = incurrentTime - inoldTime;
  if (inelapsedTime > 1000) {
    inflowRate = ((1000.0 / inelapsedTime) * inpulseCount) / calibrationFactor;
    intotalLiters += (inflowRate / 60.0);
    Serial.print("Inflow Rate: ");
    Serial.print(inflowRate);
    Serial.print(" L/min\tTotal: ");
    Serial.println(intotalLiters);
    inpulseCount = 0;
    inoldTime = incurrentTime;
  }

  // Outflow sensor
  unsigned long outcurrentTime = millis();
  unsigned long outelapsedTime = outcurrentTime - outoldTime;
  if (outelapsedTime > 1000) {
    outflowRate = ((1000.0 / outelapsedTime) * outpulseCount) / calibrationFactor;
    outtotalLiters += (outflowRate / 60.0);
    Serial.print("Outflow Rate: ");
    Serial.print(outflowRate);
    Serial.print(" L/min\tTotal: ");
    Serial.println(outtotalLiters);
    outpulseCount = 0;
    outoldTime = outcurrentTime;
  }

  // Servo control
  if (dist < 20) {
    myServo.write(0);
  } else {
    myServo.write(90);
  }

  // Firebase updates
  Firebase.RTDB.setInt(&fbdo, "/slave1/level", dist);
  Firebase.RTDB.setInt(&fbdo, "/slave1/S1I", intotalLiters);
  Firebase.RTDB.setInt(&fbdo, "/slave1/S1O", outtotalLiters);

  delay(250); // Delay between cycles
}