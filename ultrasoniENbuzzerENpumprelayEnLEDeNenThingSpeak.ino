//WiFi connection attempts occur last in the setup function, and the failure
// to connect to WiFi does not affect the execution of other parts of the code
//After every 2 minutes of WiFi connection attempts failing a comment is printed to the serial monitor:
//This code ensures that the relay turns on immediately if the water level is below 4 cm when the code starts running
//The loop will then continue to monitor the water level and adjust the relay as needed.
//The buzzer and LED are triggered whenever there is a change in the state of the relay, 
//regardless of whether it turns on or off. This ensures that the buzzer and LED react accordingly to the relay's state changes




#include <WiFi.h>
#include "ThingSpeak.h"

const int trigPin = 5;    // D5
const int echoPin = 18;   // D18
const int buzzerPin = 13; // D13
const int relayPin = 23;  // D23
const int ledPin = 33;    // D33

float tank_height = 13.5; // cm
bool buzzerTriggered = false;
bool relayState = false;

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float water_level;

const char *ssid = "Serenity"; // Replace "YourSSID" with your WiFi SSID
const char *pass = "catel43Aa"; // Replace "YourPassword" with your WiFi password

int keyIndex = 0;
WiFiClient client;

unsigned long myChannelNumber = 2077878;                 // Your ThingSpeak Channel Number
const char *myWriteAPIKey = "H1C9FOYJ9T7SXTS8"; // Your ThingSpeak Write API Key

// Function prototype
void triggerBuzzer();

void setup()
{
    Serial.begin(115200);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Initially turn off the relay

    // Initialize LEDC for LED control
    ledcSetup(0, 5000, 8); // LEDC channel 0, 5 kHz frequency, 8-bit resolution
    ledcAttachPin(ledPin, 0); // Attach LED to channel 0

    // Check water level at the beginning
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    water_level = tank_height - (duration * SOUND_SPEED / 2);

    if (water_level < 4)
    {
        digitalWrite(relayPin, LOW);
        relayState = true;
        triggerBuzzer();
        ledcWrite(0, 255); // Turn on the LED (max brightness)
    }
 Serial.print("This is a RESIDENTIAL WATER PUMPING project by PETER,BARIKK,PADDU Y21 IA EEE");
    WiFi.begin(ssid, pass);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        if (millis() - startTime > 120000) // Print a message every 2 minutes if WiFi connection attempts fail
        {
            Serial.println("WiFi connection failed!");
            startTime = millis();
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    float previouswater_level = water_level;
    water_level = tank_height - (duration * SOUND_SPEED / 2);

    Serial.print("NOTE.-->TANK water level (cm): ");
    Serial.println(water_level);

    if (water_level < 4)
    {     Serial.println("HI Team, low water level pump activated ");
        // Turn on the relay if it's not already on
        if (!relayState)
        {
            digitalWrite(relayPin, LOW);
            relayState = true;
            triggerBuzzer();
            ledcWrite(0, 255); // Turn on the LED (max brightness)
        }
    }
    else if (water_level > 9)
    { Serial.println("HI Team,the tank is full,pump going off ");
        // Turn off the relay if it's not already off
        if (relayState)
        {
            digitalWrite(relayPin, HIGH);
            relayState = false;
            triggerBuzzer();
            ledcWrite(0, 0); // Turn off the LED
        }
    }
    else
    {
        // Reset buzzer and LED
        noTone(buzzerPin);
        buzzerTriggered = false;
        ledcWrite(0, 0); // Turn off the LED
    }

    // Connect to ThingSpeak
    ThingSpeak.begin(client);

    // Write the current water level to ThingSpeak
    ThingSpeak.writeField(myChannelNumber, 2, water_level, myWriteAPIKey);

    delay(2000); // Update ThingSpeak every 2 seconds
    
    // check if the water level has 
    if (water_level != previouswater_level) {
    // If the distance has changed, write the new value to ThingSpeak as well
    ThingSpeak.writeField(myChannelNumber, 3, water_level-previouswater_level, myWriteAPIKey);
  }
}

void triggerBuzzer()
{
    if (!buzzerTriggered)
    {
        tone(buzzerPin, 1000); // Set buzzer tone to 1000 Hz
        delay(1500);
        noTone(buzzerPin);
        delay(1500);
        tone(buzzerPin, 1000); // Set buzzer tone to 1000 Hz
        delay(1500);
        noTone(buzzerPin);
        buzzerTriggered = true;
    }
}
