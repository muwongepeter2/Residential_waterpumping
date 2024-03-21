#include <WiFi.h>
#include "ThingSpeak.h"

const int trigPin = 5; // D5
const int echoPin = 18; // D18
int tank_height=14;// cm


#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;


const char *ssid = "Serenity"; // Replace "YourSSID" with your WiFi SSID
const char *pass = "catel43Aa"; // Replace "YourPassword" with your WiFi password

int keyIndex = 0;
WiFiClient client;

unsigned long myChannelNumber = 2077878; // Your ThingSpeak Channel Number
const char *myWriteAPIKey = "H1C9FOYJ9T7SXTS8"; // Your ThingSpeak Write API Key

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  WiFi.begin(ssid, pass);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  float previousDistanceCm = distanceCm;
  distanceCm = tank_height-(duration * SOUND_SPEED / 2);

 
  Serial.print("HI PETER,TANK water level (cm): ");
  Serial.println(distanceCm);
  
  
  ThingSpeak.begin(client);
  
  // Write the current distance to ThingSpeak
  ThingSpeak.writeField(myChannelNumber, 3, distanceCm, myWriteAPIKey);
  
  // Check if the distance has changed
  if (distanceCm != previousDistanceCm) {
    // If the distance has changed, write the new value to ThingSpeak as well
    ThingSpeak.writeField(myChannelNumber, 2, distanceCm, myWriteAPIKey);
  }
  
  delay(2000); // Update ThingSpeak every 5 seconds
}
