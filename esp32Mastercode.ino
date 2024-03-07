// code for the slave esp32
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>

// Function forward declaration
void sendStatusToMaster(String message);
int ultrasonic(); // Function declaration for ultrasonic sensor
int readFlowSensor(); // Function declaration for reading flow sensor

// Define your Wi-Fi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// Define master ESP32 IP address and port
IPAddress masterIP(192, 168, 1, 1); // Change this to the master ESP32's IP address
const int port = 1234; // Change this to the port you want to use

WiFiClient client;

// Define pin numbers for sensors and relay
const int flowSensorPin = 18;
const int relayPin = 22; // Change this to the pin connected to relay
const int trig = 12; // Ultrasonic sensor trig pin
const int echo = 13; // Ultrasonic sensor echo pin

// Define tank parameters
const int MaxLevel = 15; // Maximum tank height (in CM)
const int Level1 = (MaxLevel * 75) / 100;
const int Level2 = (MaxLevel * 65) / 100;
const int Level3 = (MaxLevel * 55) / 100;
const int Level4 = (MaxLevel * 35) / 100;

bool waterPresent = false;
bool tankFull = false;
bool pumpFault = false;
unsigned long lastFlowCheckTime = 0;
int lastFlowSensorValue = 0;
unsigned long lastWifiConnectAttempt = 0;
const unsigned long wifiConnectInterval = 60000; // 1 minute interval for WiFi connection attempts
const unsigned long wifiConnectTimeout = 15000; // 15 seconds timeout for WiFi connection

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Setup pins
  pinMode(flowSensorPin, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(relayPin, OUTPUT);
  pinMode(trig, OUTPUT); // Define ultrasonic sensor trig pin as output
  pinMode(echo, INPUT); // Define ultrasonic sensor echo pin as input
  
  // Disable WiFi and Bluetooth to save power during sleep
  WiFi.mode(WIFI_OFF);
  btStop();
}

void loop() {
  // Read sensor data
  int flowSensorValue = readFlowSensor(); // Read flow sensor
  int ultrasonicSensorValue = ultrasonic(); // Read ultrasonic sensor data

  // Add logic to interpret sensor data and make decisions
  // For now, let's assume water presence is detected if the sensor value is above a threshold
  waterPresent = (flowSensorValue == HIGH); // Adjust threshold as per your sensor
  
  // Interpret ultrasonic sensor data to determine tank fullness based on provided levels
  if (ultrasonicSensorValue <= MaxLevel) {
    if (ultrasonicSensorValue >= Level1) {
      tankFull = false;
    } else if (ultrasonicSensorValue >= Level2) {
      tankFull = false;
    } else if (ultrasonicSensorValue >= Level3) {
      tankFull = false;
    } else if (ultrasonicSensorValue >= Level4) {
      tankFull = true;
    }
  } else {
    // Ultrasonic sensor reading above tank max level, consider tank full
    tankFull = true;
  }
  
  // Check for low flow rate indicating pump fault or water supply cutoff
  if (waterPresent && millis() - lastFlowCheckTime > 5000) {
    int currentFlowSensorValue = readFlowSensor();
    if (currentFlowSensorValue == lastFlowSensorValue) {
      pumpFault = true;
      Serial.println("Pump fault detected");
      sendStatusToMaster("Pump fault detected");
    }
    lastFlowCheckTime = millis();
    lastFlowSensorValue = currentFlowSensorValue;
  }
  
  // Check if water is being pumped
  if (waterPresent && !tankFull && !pumpFault) {
    // If water present, tank not full, and no pump fault, turn on pump
    digitalWrite(relayPin, HIGH);
    Serial.println("Pump turned on");
    lastFlowCheckTime = millis();
    lastFlowSensorValue = flowSensorValue;
    
    // Send status to master
    sendStatusToMaster("Water present and pumping");
  } else {
    // If water not being pumped, turn off pump
    digitalWrite(relayPin, LOW);
    Serial.println("Pump turned off");
  }

  // Enter light sleep mode until conditions change
  esp_sleep_enable_timer_wakeup(30000000); // Wake up every 30 seconds (30,000,000 microseconds)
  esp_light_sleep_start();
}

void sendStatusToMaster(String message) {
  if (client.connect(masterIP, port)) {
    client.println(message);
    // Close the connection
    client.stop();
  }
}

int ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  return distance;
}

int readFlowSensor() {
  return digitalRead(flowSensorPin);
}

