#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi credentials
#define WIFI_SSID "Heyy"
#define WIFI_PASSWORD "Boobathi"

// Firebase credentials
#define API_KEY "AIzaSyCK2HEBxkp323xjv7XgFo6w2-TyHg0Rm-Q"
#define DATABASE_URL "https://aqua-sync-40a0f-default-rtdb.firebaseio.com/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Sensor pins
#define FLOW_SENSOR1_PIN 25  // Tank sensor
#define FLOW_SENSOR2_PIN 26  // Home1 sensor

// Valve control
#define RELAY_PIN1 15         // Connect relay module IN pin
#define RELAY_PIN2 27         // Connect relay module IN pin
bool valveOpen = true;

// Volume variables
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
float volume1 = 0.0;
float volume2 = 0.0;

// Timing
unsigned long lastMillis = 0;
const int interval = 1000;  // ms

// Constants
const float calibrationFactor = 7.5;  // YF-S201: ~7.5 pulses/sec per L/min

// Firebase paths
String pathVolume1 = "/aquaSync/overheadVolume";
String pathVolume2 = "/aquaSync/pipeVolume";
String pathThreshold = "/aquaSync/threshold";
String pathValveStatus = "/aquaSync/valveStatus";
String pathStatus = "/aquaSync/status";

// Helper
bool signupOK = false;

void IRAM_ATTR pulseCounter1() {
  pulseCount1++;
}

void IRAM_ATTR pulseCounter2() {
  pulseCount2++;
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected with IP: " + WiFi.localIP().toString());
}

void initFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp OK");
    signupOK = true;
  } else {
    Serial.printf("SignUp Failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void openValve() {
  digitalWrite(RELAY_PIN1, LOW);  // Active LOW relay
  digitalWrite(RELAY_PIN2, LOW);  // Active LOW relay
  valveOpen = true;
}

void closeValve() {
  digitalWrite(RELAY_PIN1, HIGH);
  digitalWrite(RELAY_PIN2, HIGH);
  valveOpen = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(FLOW_SENSOR1_PIN, INPUT_PULLUP);
  pinMode(FLOW_SENSOR2_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  openValve();

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR1_PIN), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR2_PIN), pulseCounter2, FALLING);

  connectWiFi();
  initFirebase();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= interval) {
    lastMillis = currentMillis;

    // Calculate flow in L/min and volume in L
    float flow1 = (pulseCount1 / calibrationFactor);  // L/min
    float flow2 = (pulseCount2 / calibrationFactor);

    float liter1 = (flow1 / 60.0);  // Liters in 1 sec
    float liter2 = (flow2 / 60.0);

    volume1 += liter1;
    volume2 += liter2;

    pulseCount1 = 0;
    pulseCount2 = 0;

    // Detect leakage or theft
    String flowStatus = (flow1 > 0.1 && flow2 < 0.05) ? "Leakage/Theft" : "Normal";

    // Auto-close valve if usage exceeds threshold
    if (Firebase.RTDB.getFloat(&fbdo, pathThreshold)) {
      float manualThreshold = fbdo.floatData();
      if (volume2 >= manualThreshold) {
        closeValve();
        Firebase.RTDB.setString(&fbdo, pathValveStatus, "CLOSED");
      }
    }

    // Manual override from Firebase
    if (Firebase.RTDB.getString(&fbdo, pathValveStatus)) {
      String manualStatus = fbdo.stringData();
      if (manualStatus == "OPEN" && !valveOpen) {
        openValve();
      } else if (manualStatus == "CLOSED" && valveOpen) {
        closeValve();
      }
    }

    // Update Firebase
    Firebase.RTDB.setFloat(&fbdo, pathVolume1, volume1);
    Firebase.RTDB.setFloat(&fbdo, pathVolume2, volume2);
    Firebase.RTDB.setString(&fbdo, pathStatus, flowStatus);

    // Debug log
    Serial.printf("S1: %.2f L | S2: %.2f L | Status: %s | Valve: %s\n", volume1, volume2, flowStatus.c_str(), valveOpen ? "OPEN" : "CLOSED");
  }
}
