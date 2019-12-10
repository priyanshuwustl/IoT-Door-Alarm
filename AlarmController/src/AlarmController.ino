#include "AlarmController.h"

Timer updateTemp(2000, measureTemp); //measure temperature every 2s

// PIR conference room
bool available;                  // status of conference room
unsigned long pirSensorWorkInterval = 500;
unsigned long lastPirSensorWorkTime = 0;
unsigned long pirDetermineMotionInterval = 5000;
unsigned long lastDetermineMotionTime = 0;

// publish variables
bool shouldPublish;
unsigned long publishInterval = 1500;
unsigned long lastPublishTime = 0;

// state variables (can come from UI)
bool isOn = true;
bool isStayMode = true; // if not stay, then state is away
bool isLocked = false;
bool isLockEnabled = true; // lock can be enabled/disabled
bool isAlarmEnabled = true; // alarm can be enabled/disabled
bool isCameraEnabled = false;
bool isNotificationsEnabled = true; // notifications can be enabled/disabled
bool isLockEnabledA = true; // lock can be enabled/disabled
bool isAlarmEnabledA = true; // alarm can be enabled/disabled
bool isCameraEnabledA = false;
bool isNotificationsEnabledA = true; // notifications can be enabled/disabled
bool isDoorState = true; // if not door state, then refrigerator
// state variabled (sensor data)
float lastMeasuredTemperature;

void setup() {
  // cloud functions
  bool test1 = Particle.function("publishState", publishState);
  bool test2 = Particle.function("switchOnDevice", switchOnDevice);
  bool test3 = Particle.function("enableStayMode", enableStayMode);
  bool test4 = Particle.function("lockDoorIfEnabled", lockDoorIfEnabled);
  bool test5 = Particle.function("enableLock", enableLock);
  bool test6 = Particle.function("enableAlarm", enableAlarm);
  bool test7 = Particle.function("enableCamera", enableCamera);
  bool test8 = Particle.function("enableNotifications", enableNotifications);
  bool test9 = Particle.function("changeOverallModeTo", changeOverallModeTo);
  bool test10 = Particle.function("changeSlackHook", changeSlackHook);

  setupHardware();
  setLight(false);

  // temp timer
  updateTemp.start();

  // initializing the state variables
  lastMeasuredTemperature = 0.0;
  shouldPublish = false;
  isDoorState = true;
  isStayMode = true;

  // indication
  ringAlarm();

  // for last motion time, set to CST
  Time.zone(-6);
}

//take a temperature measurement
void measureTemp() {
  float newTemp = getTempReading();
  if (newTemp != lastMeasuredTemperature) {
    lastMeasuredTemperature = newTemp;
    shouldPublish = true;
  }
}

int publishState(String s) { //publish all state information to the cloud
  String topic = "alarmController/state";
  String data = "{";
  data += "\"temperature\":";
  data += String(lastMeasuredTemperature);
  data += ",";
  data += "\"iO\":";
  data += isOn ? "true" : "false";
  data += ",";
  data += "\"iSM\":";
  data += isStayMode ? "true" : "false";
  data += ",";
  data += "\"iL\":";
  data += isLocked ? "true" : "false";
  data += ",";
  data += "\"iLE\":";
  data += isLockEnabled ? "true" : "false";
  data += ",";
  data += "\"iAE\":";
  data += isAlarmEnabled ? "true" : "false";
  data += ",";
  data += "\"iCE\":";
  data += isCameraEnabled ? "true" : "false";
  data += ",";
  data += "\"iNE\":";
  data += isNotificationsEnabled ? "true" : "false";
  data += ",";
  data += "\"iLEA\":";
  data += isLockEnabledA ? "true" : "false";
  data += ",";
  data += "\"iAEA\":";
  data += isAlarmEnabledA ? "true" : "false";
  data += ",";
  data += "\"iCEA\":";
  data += isCameraEnabledA ? "true" : "false";
  data += ",";
  data += "\"iNEA\":";
  data += isNotificationsEnabledA ? "true" : "false";
  data += ",";
  data += "\"iDS\":";
  data += isDoorState ? "true" : "false";
  data += ",";
  data += "\"sH\":";
  data += "\""+getSlackHook()+"\"";
  data += ",";
  data += "\"lMT\":";
  if (getLastMotionTime() == 0) {
    data += "\"None\"";
  }
  else {
    data += "\""+Time.format(getLastMotionTime(), TIME_FORMAT_DEFAULT)+"\"";
  }
  data += "}";
  Serial.print("data: ");
  Serial.println(data);
  shouldPublish = false;
  Particle.publish(topic, data, 60, PRIVATE);
  return 0;
}

//cloud function to edit Slack hook used for notifications
int changeSlackHook(String s) {
  editSlackHook(s);
  return 0;
}

//cloud function for light
int switchOnDevice(String s) {
  if (s == "true") {
    isOn = true;
    setLight(true);
    // START ALL THE TIMERS
    updateTemp.start();
  }
  else {
    isOn = false;
    setLight(false);
    // STOP ALL THE TIMERS
    updateTemp.stop();
  }
  shouldPublish = true;
  return 0;
}

// will need to get the settings for the respective mode
// from the UI and do things accordingly
int enableStayMode(String s) {
  if (s == "true") {
    isStayMode = true;
  }
  else {
    // away mode
    isStayMode = false;
  }

  // update the sensor booleans
  if (isStayMode) {
    setAlarmState(isAlarmEnabled);
    setCameraState(isCameraEnabled);
    setNotificationsState(isNotificationsEnabled);
  }
  else {
    setAlarmState(isAlarmEnabledA);
    setCameraState(isCameraEnabledA);
    setNotificationsState(isNotificationsEnabledA);
  }
  shouldPublish = true;
  return 0;
}

// locked/unlocked
int lockDoorIfEnabled(String s) {
  if (s.length() > 0) {
    if (s == "true") {
      isLocked = true;
      lockDoor();
    }
    else {
      isLocked = false;
      unlockDoor();
    }
  }
  shouldPublish = true;
  return 0;
}

//change lock setting
int enableLock(String s) {
  if (s.length() > 0) {
    char mode = s.charAt(0);
    String boolValue = s.substring(1);
    // if in stay state
    if (mode == 's') {
      if (boolValue == "true") {
        isLockEnabled = true;
      }
      else {
        isLockEnabled = false;
      }
    }
    // if in away state
    else if (mode == 'a') {
      if (boolValue == "true") {
        isLockEnabledA = true;
      }
      else {
        isLockEnabledA = false;
      }
    }
  }
  shouldPublish = true;
  return 0;
}

//change alarm setting
int enableAlarm(String s) {
  if (s.length() > 0) {
    char mode = s.charAt(0);
    String boolValue = s.substring(1);
    // if in stay state
    if (mode == 's') {
      if (boolValue == "true") {
        isAlarmEnabled = true;
      }
      else {
        isAlarmEnabled = false;
      }

      // send to the sensor functions accordingly
      if (isStayMode) {
        setAlarmState(isAlarmEnabled);
      }
    }
    // if in away state
    else if (mode == 'a') {
      if (boolValue == "true") {
        isAlarmEnabledA = true;
      }
      else {
        isAlarmEnabledA = false;
      }

      // send to the sensor functions accordingly
      if (!isStayMode) {
        setAlarmState(isAlarmEnabledA);
      }
    }
  }
  shouldPublish = true;
  return 0;
}

//change camera setting
int enableCamera(String s) {
  if (s.length() > 0) {
    char mode = s.charAt(0);
    String boolValue = s.substring(1);
    // if in stay state
    if (mode == 's') {
      if (boolValue == "true") {
        isCameraEnabled = true;
      }
      else {
        isCameraEnabled = false;
      }

      // send to the sensor functions accordingly
      if (isStayMode) {
        setCameraState(isCameraEnabled);
      }
    }
    // if in away state
    else if (mode == 'a') {
      if (boolValue == "true") {
        isCameraEnabledA = true;
      }
      else {
        isCameraEnabledA = false;
      }

      // send to the sensor functions accordingly
      if (!isStayMode) {
        setCameraState(isCameraEnabledA);
      }
    }
  }
  shouldPublish = true;
  return 0;
}

//change notifications setting
int enableNotifications(String s) {
  if (s.length() > 0) {
    char mode = s.charAt(0);
    String boolValue = s.substring(1);
    // if in stay state
    if (mode == 's') {
      if (boolValue == "true") {
        isNotificationsEnabled = true;
      }
      else {
        isNotificationsEnabled = false;
      }

      // send to the sensor functions accordingly
      if (isStayMode) {
        setNotificationsState(isNotificationsEnabled);
      }
    }
    // if in away state
    else if (mode == 'a') {
      if (boolValue == "true") {
        isNotificationsEnabledA = true;
      }
      else {
        isNotificationsEnabledA = false;
      }

      // send to the sensor functions accordingly
      if (!isStayMode) {
        setNotificationsState(isNotificationsEnabledA);
      }
    }
  }
  shouldPublish = true;
  return 0;
}

//switch between door and refrigerator modes
int changeOverallModeTo(String s) {
  if (s == "door") {
    isDoorState = true;
  }
  else {
    isDoorState = false;
  }
  shouldPublish = true;
  return 0;
}

void loop() {
  // publish check code
  unsigned long currentTime = millis();
  if (shouldPublish && (currentTime-lastPublishTime)>=publishInterval) {
    publishState("");
    lastPublishTime = currentTime;
    shouldPublish = false;
  }

  // buttons code below
  if (isModeButtonPressedDebounce()) {
    enableStayMode(isStayMode ? "false" : "true");
  }
  if (isPowerButtonPressedDebounce()) {
    switchOnDevice(isOn ? "false" : "true");
  }

  // PIR Code below
  // do motion sensor work every interval
  if ((currentTime-lastPirSensorWorkTime) >= pirSensorWorkInterval) {
    doMotionSensorWork();
    lastPirSensorWorkTime = currentTime;
  }

  // do determine motion every interval
  if ((currentTime-lastDetermineMotionTime) >= pirDetermineMotionInterval) {
    //Serial.println("doing motion sensor determine motion");
    determineMotion();
    lastDetermineMotionTime = currentTime;
  }
}
