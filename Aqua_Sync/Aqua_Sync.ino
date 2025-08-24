// FLOW SENSOR AND SOLENOID SMART WATER METER FOR STM32

#define FLOW_SENSOR_1_PIN D0 // From tank
#define FLOW_SENSOR_2_PIN D1 // To home 1
#define SOLENOID_VALVE_PIN1 D10 // Output to relay controlling valve
#define SOLENOID_VALVE_PIN2 D11 // Output to relay controlling valve

volatile unsigned int pulseCount1 = 0;
volatile unsigned int pulseCount2 = 0;

float calibrationFactor = 450.0; // pulses per liter (adjust to your sensor)
float waterLimitLiters = 0.5; // example daily limit for home 1
float totalHome1Liters = 0.0;

unsigned long lastMillis = 0;
const unsigned long interval = 1000; // 1 second

void countPulse1() {
  pulseCount1++;
}

void countPulse2() {
  pulseCount2++;
}

void setup() {
  Serial.begin(115200);

  pinMode(FLOW_SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(FLOW_SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(SOLENOID_VALVE_PIN1, OUTPUT);
  pinMode(SOLENOID_VALVE_PIN2, OUTPUT);
  digitalWrite(SOLENOID_VALVE_PIN1, LOW); // Valve open initially
  digitalWrite(SOLENOID_VALVE_PIN2, LOW); // Valve open initially

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_1_PIN), countPulse1, RISING);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_2_PIN), countPulse2, RISING);
}

void loop() {
  if (millis() - lastMillis >= interval) {
    noInterrupts();
    unsigned int count1 = pulseCount1;
    unsigned int count2 = pulseCount2;
    pulseCount1 = 0;
    pulseCount2 = 0;
    interrupts();

    float litersFromTank = count1 / calibrationFactor;
    float litersToHome1 = count2 / calibrationFactor;
    totalHome1Liters += litersToHome1;

    Serial.print("Tank Flow: ");
    Serial.print(litersFromTank);
    Serial.print(" L, Home 1 Flow: ");
    Serial.print(litersToHome1);
    Serial.print(" L, Total Home 1: ");
    Serial.print(totalHome1Liters);
    Serial.println(" L");

    // Leakage or Theft Detection
    if (litersFromTank > 0.05 && litersToHome1 < 0.01) {
      Serial.println("⚠️ ALERT: Possible Leakage or Theft Detected!");
      // Optional: Send alert to server or blink LED
    }

    // Limit Check
    if (totalHome1Liters >= waterLimitLiters) {
      digitalWrite(SOLENOID_VALVE_PIN1, HIGH); // Turn off water
      digitalWrite(SOLENOID_VALVE_PIN2, HIGH); // Turn off water
      Serial.println("🔒 Water limit exceeded! Valve Closed for Home 1.");
    } else {
      digitalWrite(SOLENOID_VALVE_PIN1, LOW); // Allow water
      digitalWrite(SOLENOID_VALVE_PIN2, LOW); // Allow water
    }

    lastMillis = millis();
  }
}
