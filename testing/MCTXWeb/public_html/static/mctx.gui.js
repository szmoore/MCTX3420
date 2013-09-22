/**
 * MCTX3420 2013 GUI stuff.
 */

mctx = {};
//mctx.api = location.protocol + "/" +  location.host + "/api/";
mctx.api = "http://mctx.us.to:8080/api/";
mctx.expected_api_version = 0;
mctx.sensors = {
  0 : {name : "Strain gauge 1"},
  1 : {name : "Strain gauge 2"},
  2 : {name : "Strain gauge 3"},
  3 : {name : "Strain gauge 4"},
  4 : {name : "Pressure sensor 1"},
  5 : {name : "Pressure sensor 2"}
};

mctx.actuators = {
  0 : {name : "Solenoid 1"},
  1 : {name : "Solenoid 2"},
  2 : {name : "Solenoid 3"},
  3 : {name : "Pressure regulator"}
};

function getDate(){
	document.write((new Date()).toDateString());
}

/** 
 * Populates the navigation bar
 */
$.fn.populateNavbar = function () {
  var menu = $("<ul/>", {class : "menu"});
  var sensorEntry = $("<li/>").append($("<a/>", {text : "Sensor data", href : "#"}));
  var submenu = $("<ul/>", {class : "submenu"});
  
  for (sensor in mctx.sensors) {
    var href = mctx.api + "sensors?start_time=0&format=tsv&id=" + sensor;
    submenu.append($("<li/>").append(
          $("<a/>", {text : mctx.sensors[sensor].name, 
                     href : href, target : "_blank"})
    ));
  }
  menu.append(sensorEntry.append(submenu));
  
  var actuatorEntry = $("<li/>").append($("<a/>", {text : "Actuator data", href : "#"}));
  submenu = $("<ul/>", {class : "submenu"});
  
  for (actuator in mctx.actuators) {
    var href = mctx.api + "actuators?start_time=0&format=tsv&id=" + actuator;
    submenu.append($("<li/>").append(
          $("<a/>", {text : mctx.actuators[actuator].name, 
                     href : href, target : "_blank"})
    ));
  }
  menu.append(actuatorEntry.append(submenu));  
  menu.appendTo(this);
}

/**
 * Sets the camera autoupdater
 * @returns {$.fn}
 */
$.fn.setCamera = function () {
  var loc = mctx.api + "image";
  var update = true;

  this.error(function() {
    update = false;
  });
  
  var parent = this;
  
  var updater = function() {
    if (!update) {
      alert("Cam fail");
      parent.attr("src", "");
      return;
    }
    
    parent.attr("src", loc + "#" + (new Date()).getTime());
    
    setTimeout(updater, 500);
  };
  
  updater();
  return this;
};