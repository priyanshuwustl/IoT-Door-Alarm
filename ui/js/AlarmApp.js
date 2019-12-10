var tabs = ["mainpage","sensorpage","refrigeratorpage","settingspage","camerapage"]; //add camera later
var powerButton, lockButton, stayModeButton, tempSpan, movementTimeSpan;
var stayAlarmSetting, stayLockSetting, stayCameraSetting, stayNotificationsSetting,
  awayAlarmSetting, awayLockSetting, awayCameraSetting, awayNotificationsSetting;
var slackInputBox;
//load a given tab on the screen. all others will be hidden
function loadTab(id) {
  document.body.hidden = false;
  for (var i = 0; i < tabs.length; i++) {
    document.getElementById(tabs[i]).hidden=true;
  }
  document.getElementById(id).hidden = false;
}

function loadMain() {
  loadTab("mainpage"); //load the door mode page
}

function loadSensor() {
  loadTab("sensorpage"); //load sensor data page
}

function loadRefrigerator() {
  loadTab("refrigeratorpage"); //load the refrigerator page
}

function loadSettings() {
  loadTab("settingspage"); //load the settings page
}

function lockButtonPress() { //listener for when the lock button is pressed, moves the lock when enabled
  console.log("app lock function");
  alarm.setLock();
}

function stayModeButtonPress() { //listener for stay mode button being pressed. calls function to toggle mode
  if (alarm.stayMode) {
    alarm.setStayMode(false);
  }
  else alarm.setStayMode(true);
}

function powerButtonPress() { //listener for light button being pressed.
  if (!alarm.power) {
    alarm.setPower(true);
  }
  else {
    alarm.setPower(false);
  }
}

function refrigeratorModeButtonPress() { //listener for link between door and refrigerator pages
  alarm.doorState ? alarm.setDoorState("door") : alarm.setDoorState("fridge");
}

//listener functions for check boxes in settings
function stayAlarmChange() {
  alarm.setAlarmEnabled(true);
}

function stayLockChange() {
  alarm.setLockEnabled(true);
}

function stayCameraChange() {
  alarm.setCameraEnabled(true);
}

function stayNotificationsChange() {
  alarm.setNotificationsEnabled(true);
}

function awayAlarmChange() {
  alarm.setAlarmEnabled(false);
}

function awayLockChange() {
  alarm.setLockEnabled(false);
}

function awayCameraChange() {
  alarm.setCameraEnabled(false);
}

function awayNotificationsChange() {
  alarm.setNotificationsEnabled(false);
}

function slackHookChange() {
  alarm.setSlackHook(slackInputBox.value);
}

function stateUpdate(newState) { //update UI with state of alarm objects
  for (var i = 0; i < powerButton.length; i++) {
    powerButton[i].innerText = newState.power ? "Turn Off Light" : "Turn On Light";
  }
  for (var i = 0; i < lockButton.length; i++) {
    lockButton[i].innerText = newState.isLocked ? "Unlock" : "Lock";
  }
  for (var i = 0; i < stayModeButton.length; i++) {
    stayModeButton[i].innerText = newState.stayMode ? "Go To Away Mode" : "Go To Stay Mode";
  }
  for (var i = 0; i < tempSpan.length; i++) {
    tempSpan[i].innerText = newState.temperature;
  }
  for (var i = 0; i < movementTimeSpan.length; i++) {
    movementTimeSpan[i].innerText = newState.lastMotionTime;
  }

  stayAlarmSetting.checked = newState.alarmEnabled;
  stayLockSetting.checked = newState.lockEnabled;
  stayCameraSetting.checked = newState.cameraEnabled;
  stayNotificationsSetting.checked = newState.notificationsEnabled;
  awayAlarmSetting.checked = newState.alarmEnabledA;
  awayLockSetting.checked = newState.lockEnabledA;
  awayCameraSetting.checked = newState.cameraEnabledA;
  awayNotificationsSetting.checked = newState.notificationsEnabledA;
}

//listener for page being loaded
document.addEventListener("DOMContentLoaded", function(event) {

  //variables
  var linkToSensorPage = document.getElementsByClassName("viewSensorsButton");
  var linkToMainPage = document.getElementsByClassName("homeButton");
  var linkToRefrigeratorPage = document.getElementsByClassName("refrigeratorModeButton");
  var linkToSettingsPage = document.getElementsByClassName("settingsButton");
  lockButton = document.getElementsByClassName("lockButton");
  stayModeButton = document.getElementsByClassName("stayModeButton");
  powerButton = document.getElementsByClassName("powerButton");
  tempSpan = document.getElementsByClassName("tempDataStatus");
  movementTimeSpan = document.getElementsByClassName("lastMovementStatus");

  stayAlarmSetting = document.getElementById("stayAlarmSetting");
  stayLockSetting = document.getElementById("stayLockSetting");
  stayCameraSetting = document.getElementById("stayCameraSetting");
  stayNotificationsSetting = document.getElementById("stayNotificationsSetting");

  awayAlarmSetting = document.getElementById("awayAlarmSetting");
  awayLockSetting = document.getElementById("awayLockSetting");
  awayCameraSetting = document.getElementById("awayCameraSetting");
  awayNotificationsSetting = document.getElementById("awayNotificationsSetting");

  slackInputBox = document.getElementById("slackInputBox");

  //listeners
  for (var i = 0; i< linkToMainPage.length; i++) {
    linkToMainPage[i].addEventListener("click",loadMain);
  }

  for (var i = 0; i < linkToSensorPage.length; i++) {
    linkToSensorPage[i].addEventListener("click",loadSensor);
  }

  for (var i = 0; i < linkToRefrigeratorPage.length; i++) {
    linkToRefrigeratorPage[i].addEventListener("click",loadRefrigerator);
  }

  for (var i = 0; i < linkToSettingsPage.length; i++) {
    linkToSettingsPage[i].addEventListener("click",loadSettings);
  }

  for (var i = 0; i < lockButton.length; i++) {
    lockButton[i].addEventListener("click",lockButtonPress);
  }

  for (var i = 0; i < stayModeButton.length; i++) {
    stayModeButton[i].addEventListener("click",stayModeButtonPress);
  }

  for (var i = 0; i < powerButton.length; i++) {
    powerButton[i].addEventListener("click",powerButtonPress);
  }

  stayAlarmSetting.addEventListener("change", stayAlarmChange);
  stayLockSetting.addEventListener("change", stayLockChange);
  stayCameraSetting.addEventListener("change", stayCameraChange);
  stayNotificationsSetting.addEventListener("change", stayNotificationsChange);

  awayAlarmSetting.addEventListener("change", awayAlarmChange);
  awayLockSetting.addEventListener("change", awayLockChange);
  awayCameraSetting.addEventListener("change", awayCameraChange);
  awayNotificationsSetting.addEventListener("change", awayNotificationsChange);

  slackInputBox.addEventListener("change", slackHookChange);

  alarm.setStateChangeListener(stateUpdate);
  alarm.setup();
  if (alarm.isDoorState) {
    loadMain();
  } else loadRefrigerator();
})
