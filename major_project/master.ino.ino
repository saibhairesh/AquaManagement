const int motorpin = 14; // Motor control pin

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Wi-Fi credentials
#define WIFI_SSID "Rama Aditya"
#define WIFI_PASSWORD "123456789"

// Firebase project credentials
#define API_KEY "AIzaSyAc_vIL0GF_v3AnZsumUxW1nfsFb3Dg9qk"
#define DATABASE_URL "https://projectgriet-d4959-default-rtdb.firebaseio.com/"
#define USER_EMAIL "projectgriet25@gmail.com"
#define USER_PASSWORD "projectgriet"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Timer variable
unsigned long sendDataPrevMillis = 0;

// Need variables
bool need1 = false, need2 = false, need3 = false, need4 = false;

void setup() {
  Serial.begin(115200);

  // Configure motor pin as output
  pinMode(motorpin, OUTPUT);
  digitalWrite(motorpin, LOW);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Assign user credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign token status callback function
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
}

void loop() {
  // Check Firebase readiness and timer
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Read and print need values from Firebase
    if (Firebase.RTDB.getint(&fbdo, F("/slave1/need"))) {
      need1 = fbdo.to<bool>();
      Serial.printf("Need1: %s\n", need1 ? "true" : "false");
    } else {
      Serial.printf("Error reading Need1: %s\n", fbdo.errorReason().c_str());
    }

    if (Firebase.RTDB.getint(&fbdo, F("/slave2/need"))) {
      need2 = fbdo.to<bool>();
      Serial.printf("Need2: %s\n", need2 ? "true" : "false");
    } else {
      Serial.printf("Error reading Need2: %s\n", fbdo.errorReason().c_str());
    }

    if (Firebase.RTDB.getint(&fbdo, F("/slave3/need"))) {
      need3 = fbdo.to<bool>();
      Serial.printf("Need3: %s\n", need3 ? "true" : "false");
    } else {
      Serial.printf("Error reading Need3: %s\n", fbdo.errorReason().c_str());
    }

    if (Firebase.RTDB.getint(&fbdo, F("/slave4/need"))) {
      need4 = fbdo.to<bool>();
      Serial.printf("Need4: %s\n", need4 ? "true" : "false");
    } else {
      Serial.printf("Error reading Need4: %s\n", fbdo.errorReason().c_str());
    }

    // Control the motor based on conditions
    if (need1 || need2 || need3 || need4) {
      digitalWrite(motorpin, HIGH);
      Serial.println("Motor ON");
    } else {
      digitalWrite(motorpin, LOW);
      Serial.println("Motor OFF");
    }
  }
}