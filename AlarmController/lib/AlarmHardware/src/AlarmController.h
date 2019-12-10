#include"Arduino.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"

/**
 * Setup the alarm hardware (all I/O should be configured here)
 *
 * This routine should be called only once from setup()
 */
void setupHardware();

void setLight(boolean on);

//************************SETTERS************************************************************************

void setAlarmState(boolean on);

void setCameraState(boolean on);

void setNotificationsState(boolean on);

//************************TEMPERATURE STUFF************************************************************************

float getTempReading();

//************************PIR SENSOR STUFF************************************************************************

void doMotionSensorWork();

void determineMotion();

void editSlackHook(String s);

String getSlackHook();

time_t getLastMotionTime();

//************************BUTTON STUFF************************************************************************

boolean isPowerButtonPressed();

boolean isModeButtonPressed();

boolean isModeButtonPressedDebounce();

boolean isPowerButtonPressedDebounce();

//************************BUZZER STUFF************************************************************************

void stopAlarm();

void ringAlarm();

//************************SERVO STUFF************************************************************************

void unlockDoor();

void lockDoor();
