var myParticleAccessToken = "";
var topic = "alarmController/state";
var myDeviceId = "";

//function to take in JSON data from the photon and change state accordingly
function newAlarmEvent(objectContainingData) {
  myObj = JSON.parse(objectContainingData.data);
  // console.log("myObj = " + myObj.data);
  alarm.power = myObj.iO;
  console.log("alarm.power is: " + alarm.power);
  alarm.stayMode = myObj.iSM;
  alarm.lockEnabled = myObj.iLE;
  alarm.isLocked = myObj.iL;
  alarm.alarmEnabled = myObj.iAE;
  alarm.cameraEnabled = myObj.iCE;
  alarm.notificationsEnabled = myObj.iNE;
  alarm.lockEnabledA = myObj.iLEA;
  alarm.alarmEnabledA = myObj.iAEA;
  alarm.cameraEnabledA = myObj.iCEA;
  alarm.notificationsEnabledA = myObj.iNEA;
  alarm.isDoorState = myObj.iDS;
  alarm.temperature = myObj.temperature;
  alarm.slackHook = myObj.sH;
  alarm.lastMotionTime = myObj.lMT;
  alarm.stateChange();
}

//object to model the hardware
var alarm = {
  //particle API
  particle: null,
  //state variables
  power: true,
  stayMode: false,
  lockEnabled: false,
  isLocked: false,
  alarmEnabled: false,
  cameraEnabled: false,
  notificationsEnabled: false,
  lockEnabledA: false,
  alarmEnabledA: false,
  cameraEnabledA: false,
  notificationsEnabledA: false,
  isDoorState: true,
  temperature: 0,
  lastMotionTime: "",
  slackHook: null,

  //listener function for state changes
  stateChangeListener: null,

  //set the light on or off
  setPower: function(on) {
    console.log("setPower calling with value: " + on);
    var functionData = {
      deviceId: myDeviceId,
      name: "switchOnDevice",
      argument: on ? "true" : "false",
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("switchOnDevice success with val " + on)}, function() {console.log("switchOnDevice fail")});
    this.stateChange();
  },

  //switch between Stay/Away mode
  setStayMode: function(stay) {
    var functionData = {
      deviceId: myDeviceId,
      name: "enableStayMode",
      argument: stay ? "true" : "false",
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("enableStayMode success")}, function() {console.log("enableStayMode fail")});
    this.stateChange();
  },

  //lock or unlock
  setLock: function() {
    console.log("model lock function");
    var args;
    var enabled = alarm.stayMode ? alarm.lockEnabled : alarm.lockEnabledA;
    console.log("enabled: " + enabled);
    if (enabled) args = alarm.isLocked ? "false" : "true";
    else args = "false";
    console.log("args: " + args);
    var functionData = {
      deviceId: myDeviceId,
      name: "lockDoorIfEnabled",
      argument: args,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("lockDoor success")}, function() {console.log("lockDoor fail")});
    this.stateChange();
  },

  //enable or disable the lock in either stay or away mode
  setLockEnabled: function(stay) {
    var args = stay ? "s" : "a";
    if (stay) args += alarm.lockEnabled ? "false" : "true";
    else args += alarm.lockEnabledA ? "false" : "true";
    var functionData = {
      deviceId: myDeviceId,
      name: "enableLock",
      argument: args,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("lock enabled")}, function() {console.log("lock enable fail")});
    this.stateChange();
  },

  //enable or disable the alarm in either stay or away mode
  setAlarmEnabled: function(stay) {
    var args = stay ? "s" : "a";
    if (stay) args += alarm.alarmEnabled ? "false" : "true";
    else args += alarm.alarmEnabledA ? "false" : "true";
    var functionData = {
      deviceId: myDeviceId,
      name: "enableAlarm",
      argument: args,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("alarm enabled")}, function() {console.log("alarm enable fail")});
  },

  //enable or disable the camera in either stay or away mode
  setCameraEnabled: function(stay) {
    var args = stay ? "s" : "a";
    if (stay) args += alarm.cameraEnabled ? "false" : "true";
    else args += alarm.cameraEnabledA ? "false" : "true";
    var functionData = {
      deviceId: myDeviceId,
      name: "enableCamera",
      argument: args,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("camera enabled")}, function() {console.log("camera enable fail")});
  },

  //enable or disable the notifications in either stay or away mode
  setNotificationsEnabled: function(stay) {
    var args = stay ? "s" : "a";
    if (stay) args += alarm.notificationsEnabled ? "false" : "true";
    else args += alarm.notificationsEnabledA ? "false" : "true";
    var functionData = {
      deviceId: myDeviceId,
      name: "enableNotifications",
      argument: args,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {console.log("notifications enabled")}, function() {console.log("notifications enable fail")});
  },

  //switch between door and refrigerator mode
  setDoorState: function(door) {
    var functionData = {
      deviceId: myDeviceId,
      name: "changeOverallModeTo",
      argument: door ? "door" : "fridge",
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {"setDoorState success"}, function() {"setDoorState fail"});
    this.stateChange();
  },

  //change the slack hook associated with the alarm
  setSlackHook: function(val) {
    console.log(val);
    var functionData = {
      deviceId: myDeviceId,
      name: "changeSlackHook",
      argument: val,
      auth: myParticleAccessToken
    }
    particle.callFunction(functionData).then(function() {}, function() {});
    this.stateChange();
  },

  //change the listener function being used
  setStateChangeListener: function(aListener) {
    this.stateChangeListener = aListener;
    this.stateChange();
  },

  //calls listener function and inputs current state
  stateChange: function() {
    var callingObject = this;
    if (this.stateChangeListener) {
      var state = {
        power: callingObject.power,
        stayMode: callingObject.stayMode,
        lockEnabled: callingObject.lockEnabled,
        isLocked: callingObject.isLocked,
        alarmEnabled: callingObject.alarmEnabled,
        cameraEnabled: callingObject.cameraEnabled,
        notificationsEnabled: callingObject.notificationsEnabled,
        lockEnabledA: callingObject.lockEnabledA,
        alarmEnabledA: callingObject.alarmEnabledA,
        cameraEnabledA: callingObject.cameraEnabledA,
        notificationsEnabledA: callingObject.notificationsEnabledA,
        isDoorState: callingObject.isDoorState,
        temperature: callingObject.temperature,
        slackHook: callingObject.slackHook,
        lastMotionTime: callingObject.lastMotionTime
        //ADD ALL ALARM STATE VARIABLES
      };
      console.log("state from alarm.stateChange(): " + state.power);
      callingObject.stateChangeListener(state);
    }
  },

  setup: function() {
      // Create a particle object
      particle = new Particle();

      // Get ready to subscribe to the event stream
      function onSuccess(stream) {
        // DONE:  This will "subscribe' to the stream and get the state"
        console.log("getEventStream success")
        stream.on('event', newAlarmEvent)

        // TODO: Get the initial state.  Call your function that will publish the state
        // NOTE: This is here in the callback to the subscribe --- it will request the state
        //       once successbully subscribed.
        console.log(myParticleAccessToken)
        var functionData = {
           deviceId: myDeviceId,
           name: "publishState",
           argument: "",
           auth: myParticleAccessToken
         };
         function onSuccess(e) {
           console.log("success")
         }
         function onFailure(e) {
           console.dir(e);
         }
         particle.callFunction(functionData).then(onSuccess, onFailure);


      }
      function onFailure(e) { console.log("getEventStream call failed")
                              console.dir(e) }

      // Subscribe to the stream
      particle.getEventStream( { name: topic, auth: myParticleAccessToken, deviceId: myDeviceId }).then(onSuccess, onFailure);
  }
}
