/**
 * MCTX3420 2013 GUI stuff.
 */

mctx = {};
mctx.api = location.protocol + "//" +  location.host + "/api/";
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

mctx.strain_gauges = {};
mctx.strain_gauges.ids = [0, 1, 2, 3];
mctx.strain_gauges.time_limit = 20;

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
  var url = mctx.api + "image";
  var update = true;

  //Stop updating if we can't retrieve an image!
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
    
    parent.attr("src", url + "#" + (new Date()).getTime());
    
    setTimeout(updater, 500);
  };
  
  updater();
  return this;
};

$.fn.setStrainGraphs = function () {
  var sensor_url = mctx.api + "sensors";
  var graphdiv = this;
  
  var updater = function () {
    var time_limit = mctx.strain_gauges.time_limit;
    var responses = new Array(mctx.strain_gauges.ids.length);
    
    for (var i = 0; i < mctx.strain_gauges.ids.length; i++) {
      var parameters = {id : i, start_time: -time_limit};
      responses[i] = $.ajax({url : sensor_url, data : parameters});
    }
    
    $.when.apply(this, responses).then(function () {
      var data = new Array(arguments.length);
      for (var i = 0; i < arguments.length; i++) {
        var raw_data = arguments[i][0].data;
        data[i] = raw_data;
      }
      $.plot(graphdiv, data);
      setTimeout(updater, 500);
    }, function () {alert("boo");});
  };
  
  updater();
  return this;
};