#include "Arduino.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"

#include"AlarmController.h"

// pins
const int ledPin = D5;
const int powerPin = D1;
const int modePin = D2;
const int buzzerPin = D7;
const int servoPin = D3;
int inputPin = D0;               // choose the PIR sensor pin

// temperature sensor
OneWire ds = OneWire(D4);
DallasTemperature sensors(&ds);
unsigned long lastUpdate = 0;
float lastTemp;

// PIR Sensor
int motionCounter = 0;           // variable to count motion events
String pirVal = "conf_avail";
bool didDoorMove = false;
String slackURL = "";
String slackHookData = String::format("{ \"slackURL\": \"%s\"}", slackURL.c_str());
time_t lastMotionTime = 0;

// Servo Stuff
Servo servo;
int unlockPosition = 0;
int lockPosition = 180;

// DEBOUNCE variables
boolean lastModeState = false;						// The state of the button in the previous loop iteration
boolean modeAlreadyDone = false;					// Whether or not the button functionality has taken place for this press
unsigned long lastModePressTime = 0;				// The millisecond time that the button last started being pressed

boolean lastPowerState = false;
boolean powerAlreadyDone = false;
unsigned long lastPowerPressTime = 0;
const unsigned long DEBOUNCE = 100;

// settings state
boolean alarmVal = true;
boolean cameraVal = false;
boolean notificationsVal = true;

/**
 * Setup the alarm hardware (all I/O should be configured here)
 *
 * This routine should be called only once from setup()
 */
void setupHardware() {
  Serial.begin(9600);

  // LEDs
  pinMode(ledPin, OUTPUT);

  // buttons
  pinMode(powerPin, INPUT_PULLUP);
  pinMode(modePin, INPUT_PULLUP);

  // PIR motion sensor
  pinMode(inputPin, INPUT);

  // Piezo buzzer
  pinMode(buzzerPin, OUTPUT);

  // servo
  servo.attach(servoPin);
  servo.write(unlockPosition);

  // Start up the temp sensor library
  sensors.begin();
}

void setLight(boolean on) {
  digitalWrite(ledPin, on ? HIGH : LOW);
}

//************************SETTERS************************************************************************
void setAlarmState(boolean on) {
  alarmVal = on;
}

void setCameraState(boolean on) {
  cameraVal = on;
}

void setNotificationsState(boolean on) {
  notificationsVal = on;
}

//************************TEMPERATURE STUFF************************************************************************
float getTempReading() {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(sensors.getTempFByIndex(0));
  return sensors.getTempFByIndex(0);
}

//************************PIR SENSOR STUFF************************************************************************
void doMotionSensorWork() {
  if (digitalRead(inputPin) == HIGH) {  // check if the input is HIGH
    motionCounter++;              // increment motion counter
  }
  // delay replaced by delta timing in main code
  // delay(500);
}

void determineMotion() {    // this function determines if there's motion
    Serial.println("motionCounter: " + String(motionCounter));
    if(motionCounter < 3) { // if very little motion was detected
        if(didDoorMove == true && notificationsVal) { // only publish if the status changed
          // Serial.println("door stopped");
          Particle.publish("door_stopped", slackHookData, PRIVATE); // publish
        }
        didDoorMove = false; // set the status to available
    } else if (motionCounter >= 3) {
        // Serial.println("Came here");
        lastMotionTime = Time.now();
        if(didDoorMove == false) { // only publish if the status changed
          if (notificationsVal) {
            // Serial.println("door moved publish here");
            Particle.publish("door_moved", slackHookData, PRIVATE); // publish
          }
          if (alarmVal) {
            // Serial.println("Alarm here");
            ringAlarm();
          }
        }
        didDoorMove = true; // set the status to in use
    }
    motionCounter = 0; // reset motion counter
}

void editSlackHook(String s) {
  // change the variable slackURL with thing after:
  // https://hooks.slack.com/services/
  // Need to pass {{{slackURL}}} in:
  // https://hooks.slack.com/services/{{{hookURL}}}
  String startingSlack = "https://hooks.slack.com/services/";
  if (s.startsWith(startingSlack)) {
    slackURL = s.substring(startingSlack.length());
    slackHookData = String::format("{ \"slackURL\": \"%s\"}", slackURL.c_str());
  }
}

String getSlackHook() {
  return "https://hooks.slack.com/services/" + slackURL;
}

time_t getLastMotionTime() {
  return lastMotionTime;
}

//************************BUTTON STUFF************************************************************************
bool isPowerButtonPressed() {
  return !digitalRead(powerPin);
}

bool isModeButtonPressed() {
  return !digitalRead(modePin);
}

bool isModeButtonPressedDebounce() { //Debounce code taken from Piazza post
  unsigned long currentMillis = millis();				// Time at the start of the loop iteration
  bool isPressed = isModeButtonPressed();				// The state of the button in the current loop iteration abbreviated for readability

  // Checks if the button is pressed, then checks if the debounce delay has passed, then finally checks if the button functionality has been performed already
  if(isPressed && lastModePressTime + DEBOUNCE <= currentMillis && !modeAlreadyDone) {
    // BUTTON PRESS FUNCTIONALITY GOES HERE
    modeAlreadyDone = true;
    return true;
  }

  // Checks if the button has been released (gone from pressed to not pressed between the previous and the current iterations)
  else if(!isPressed && lastModeState) {
    lastModeState = false;
    modeAlreadyDone = false;
  }

  // Checks if the button has been pressed (gone from not pressed to pressed between the previous and the current iterations)
  else if(isPressed && !lastModeState) {
    lastModeState = true;
    lastModePressTime = currentMillis;
  }
  return false;
}

bool isPowerButtonPressedDebounce() { //Debounce code taken from Piazza post
  unsigned long currentMillis = millis();				// Time at the start of the loop iteration
  bool isPressed = isPowerButtonPressed();				// The state of the button in the current loop iteration abbreviated for readability

  // Checks if the button is pressed, then checks if the debounce delay has passed, then finally checks if the button functionality has been performed already
  if(isPressed && lastPowerPressTime + DEBOUNCE <= currentMillis && !powerAlreadyDone) {
    // BUTTON PRESS FUNCTIONALITY GOES HERE
    powerAlreadyDone = true;
    return true;
  }

  // Checks if the button has been released (gone from pressed to not pressed between the previous and the current iterations)
  else if(!isPressed && lastPowerState) {
    lastPowerState = false;
    powerAlreadyDone = false;
  }

  // Checks if the button has been pressed (gone from not pressed to pressed between the previous and the current iterations)
  else if(isPressed && !lastPowerState) {
    lastPowerState = true;
    lastPowerPressTime = currentMillis;
  }
  return false;
}

//************************BUZZER STUFF************************************************************************
Timer stopper(1000, stopAlarm, true);
void stopAlarm() {
  digitalWrite(buzzerPin, LOW);
}
void ringAlarm() {
  digitalWrite(buzzerPin, HIGH);
  stopper.start();
}

//************************SERVO STUFF************************************************************************
void unlockDoor() {
  servo.write(unlockPosition);
}

void lockDoor() {
  servo.write(lockPosition);
}
