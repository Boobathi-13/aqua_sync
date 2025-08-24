// #include <WiFi.h>
// #include <Firebase_ESP_Client.h>

// // Include required helper headers (provided by library)
// #include "addons/TokenHelper.h"
// #include "addons/RTDBHelper.h"

// // WiFi credentials
// #define WIFI_SSID "Heyy"
// #define WIFI_PASSWORD "Boobathi"

// // Firebase credentials
// #define DATABASE_URL "https://aqua-sync-40a0f-default-rtdb.firebaseio.com/"
// #define API_KEY "AIzaSyCK2HEBxkp323xjv7XgFo6w2-TyHg0Rm-Q"

// // Firebase objects
// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;

// // Global flag for signup
// bool signupOK = false;

// // Sample data
// float pipeVolume = 0.0;
// float overheadVolume = 0.0;
// String valveStatus = "Open";
// String statusMessage = "Normal";

// // Connect to Wi-Fi
// void connectWiFi() {
// WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
// Serial.print("Connecting to WiFi");
// while (WiFi.status() != WL_CONNECTED) {
// delay(500);
// Serial.print(".");
// }
// Serial.println();
// Serial.print("Connected! IP: ");
// Serial.println(WiFi.localIP());
// }

// // Initialize Firebase
// void initFirebase() {
// config.api_key = API_KEY;
// config.database_url = DATABASE_URL;

// if (Firebase.signUp(&config, &auth, "", "")) {
// Serial.println("✅ Firebase Signup Success");
// signupOK = true;
// } else {
// Serial.printf("❌ Firebase Signup Failed: %s\n", config.signer.signupError.message.c_str());
// }

// config.token_status_callback = tokenStatusCallback; // Optional

// Firebase.begin(&config, &auth);
// Firebase.reconnectWiFi(true);
// }

// void updateFirebase() {
// if (!signupOK) return;

// // Write float values
// if (Firebase.RTDB.setFloat(&fbdo, "/aquaSync/pipeVolume", pipeVolume)) {
// Serial.println("✅ pipeVolume updated");
// } else {
// Serial.println("❌ pipeVolume update failed: " + fbdo.errorReason());
// }

// if (Firebase.RTDB.setFloat(&fbdo, "/aquaSync/overheadVolume", overheadVolume)) {
// Serial.println("✅ overheadVolume updated");
// } else {
// Serial.println("❌ overheadVolume update failed: " + fbdo.errorReason());
// }

// // Write string values
// if (Firebase.RTDB.setString(&fbdo, "/aquaSync/valveStatus", valveStatus)) {
// Serial.println("✅ valveStatus updated");
// } else {
// Serial.println("❌ valveStatus update failed: " + fbdo.errorReason());
// }

// if (Firebase.RTDB.setString(&fbdo, "/aquaSync/status", statusMessage)) {
// Serial.println("✅ status updated");
// } else {
// Serial.println("❌ status update failed: " + fbdo.errorReason());
// }
// }

// void setup() {
// Serial.begin(115200);
// connectWiFi();
// initFirebase();
// }

// void loop() {
// // Simulate data
// pipeVolume += 0.05;
// overheadVolume += 0.03;

// // Update Firebase every 5 seconds
// static unsigned long lastUpdate = 0;
// if (millis() - lastUpdate > 5000) {
// updateFirebase();
// lastUpdate = millis();
// }
// }



#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Firebase Helper headers
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "Heyy"
#define WIFI_PASSWORD "Boobathi"

// Firebase credentials
#define DATABASE_URL "https://aqua-sync-40a0f-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyCK2HEBxkp323xjv7XgFo6w2-TyHg0Rm-Q"

// Flow sensor pins
#define FLOW_SENSOR_OVERHEAD 13
#define FLOW_SENSOR_PIPE 14

// Relay control pin
#define RELAY_PIN 25

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Flags and variables
bool signupOK = false;

// Flow sensor data
volatile int pulseCountOverhead = 0;
volatile int pulseCountPipe = 0;
float flowRateOverhead = 0.0, flowRatePipe = 0.0;
float volumeOverhead = 0.0, volumePipe = 0.0;

    // ✅ Get valve status from Firebase
    if (Firebase.RTDB.getString(&fbdo, "/aquaSync/valveStatus")) {
      valveStatus = fbdo.stringData();
      Serial.println("🔄 Valve Status from Firebase: " + valveStatus);

      if (valveStatus == "Open") {
        digitalWrite(RELAY_PIN, LOW); // Relay OFF → Valve Open
        Serial.println("✅ Valve OPENED (Relay LOW)");
      } else if (valveStatus == "Close") {
        digitalWrite(RELAY_PIN, HIGH); // Relay ON → Valve Closed
        Serial.println("✅ Valve CLOSED (Relay HIGH)");
String statusMessage = "Normal";

// Timing variables
unsigned long previousMillis = 0;
const unsigned long interval = 5000; // 5 seconds

// Interrupt Service Routines
void IRAM_ATTR countPulseOverhead() {
  pulseCountOverhead++;
}

void IRAM_ATTR countPulsePipe() {
  pulseCountPipe++;
}

// WiFi Connection
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
}

// Firebase Initialization
void initFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("✅ Firebase Signup Success");
    signupOK = true;
  } else {
    Serial.printf("❌ Firebase Signup Failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// Firebase Update
void updateFirebase() {
  if (!signupOK) return;

  // Send volumes
  if (Firebase.RTDB.setFloat(&fbdo, "/aquaSync/pipeVolume", volumePipe))
    Serial.println("✅ pipeVolume updated");
  else
    Serial.println("❌ pipeVolume update failed: " + fbdo.errorReason());

  if (Firebase.RTDB.setFloat(&fbdo, "/aquaSync/overheadVolume", volumeOverhead))
    Serial.println("✅ overheadVolume updated");
  else
    Serial.println("❌ overheadVolume update failed: " + fbdo.errorReason());

  // Send statuses
  if (Firebase.RTDB.setString(&fbdo, "/aquaSync/valveStatus", valveStatus))
    Serial.println("✅ valveStatus updated");
  else
    Serial.println("❌ valveStatus update failed: " + fbdo.errorReason());

  if (Firebase.RTDB.setString(&fbdo, "/aquaSync/status", statusMessage))
    Serial.println("✅ status updated");
  else
    Serial.println("❌ status update failed: " + fbdo.errorReason());
}

void setup() {
  Serial.begin(115200);

  // Pin setup
  pinMode(FLOW_SENSOR_OVERHEAD, INPUT_PULLUP);
  pinMode(FLOW_SENSOR_PIPE, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Valve open

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_OVERHEAD), countPulseOverhead, RISING);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIPE), countPulsePipe, RISING);

  // Setup WiFi and Firebase
  connectWiFi();
  initFirebase();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Disable interrupts to read pulse counts safely
    noInterrupts();
    int overheadPulses = pulseCountOverhead;
    int pipePulses = pulseCountPipe;
    pulseCountOverhead = 0;
    pulseCountPipe = 0;
    interrupts();

    // Calculate flow rate (example: 7.5 pulses per liter/min)
    flowRateOverhead = (overheadPulses / 7.5) * 1.0; // L/min
    flowRatePipe = (pipePulses / 7.5) * 1.0;         // L/min

    // Convert to volume (L) for the interval (5 sec)
    volumeOverhead += (flowRateOverhead / 60.0) * (interval / 1000.0);
    volumePipe += (flowRatePipe / 60.0) * (interval / 1000.0);

    Serial.printf("Overhead: %.2f L, Pipe: %.2f L\n", volumeOverhead, volumePipe);

    // Send data to Firebase
    updateFirebase();

      }
    } else {
      Serial.println("❌ Failed to get valveStatus: " + fbdo.errorReason());
    }
  }
}