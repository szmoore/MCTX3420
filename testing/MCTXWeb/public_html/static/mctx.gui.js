/**
 * MCTX3420 2013 GUI stuff.
 */

mctx = {};
mctx.api = location.protocol + "//" +  location.host + "/api/";
mctx.expected_api_version = 0;
mctx.key = undefined;
mctx.has_control = false;

mctx.return_codes = {
  "1" : "Ok",
  "-1" : "General error",
  "-2" : "Unauthorized",
  "-3" : "Not running",
  "-4" : "Already exists"
};

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

/**
 * Writes the current date to wherever it's called.
 */
function getDate(){
	document.write((new Date()).toDateString());
}

/**
 * Populates a submenu of the navigation bar
 * @param {string} header The header
 * @param {object} items An object representing the submenu items
 * @param {function} translator A function that translates an object item
 *                              into a text and href.
 * @returns {$.fn} Itself
 */
$.fn.populateSubmenu = function(header, items, translator) {
  var submenuHeader = $("<li/>").append($("<a/>", {text : header, href : "#"}));
  var submenu = $("<ul/>", {"class" : "submenu"});
  
  for (var item in items) {
    var info = translator(item, items);
    submenu.append($("<li/>").append(
          $("<a/>", {text : info.text, 
                     href : info.href, target : "_blank"})
    ));
  }
  
  this.append(submenuHeader.append(submenu));
  return this;
};

/** 
 * Populates the navigation bar
 */
$.fn.populateNavbar = function () {
  var menu = $("<ul/>", {"class" : "menu"});
  var sensorTranslator = function(item, items) {
    var href = mctx.api + "sensors?start_time=0&format=tsv&id=" + item;
    return {text : items[item].name, href : href};
  };
  var actuatorTranslator = function(item, items) {
    var href = mctx.api + "actuators?start_time=0&format=tsv&id=" + item;
    return {text : items[item].name, href : href};
  };
  
  menu.populateSubmenu("Sensor data", mctx.sensors, sensorTranslator);
  menu.populateSubmenu("Actuator data", mctx.actuators, actuatorTranslator);
  menu.appendTo(this);
  return this;
}

/**
 * Sets the camera autoupdater
 * @returns {$.fn}
 */
$.fn.setCamera = function () {
  var url = mctx.api + "image";  //http://beaglebone/api/image
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
    
    setTimeout(updater, 1000);
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
        var pruned_data = [];
        var step = ~~(raw_data.length/100);
        for (var j = 0; j < raw_data.length; j += step)
          pruned_data.push(raw_data[j]); 
        data[i] = pruned_data;
      }
      $.plot(graphdiv, data);
      setTimeout(updater, 500);
    }, function () {alert("It crashed");});
  };
  
  updater();
  return this;
};

$.fn.login = function () {
  var username = this.find("input[name='username']").val();
  var password = this.find("input[name='pass']").val();
  var force = this.find("input[name='force']").is(":checked");
  var url = mctx.api + "control";
  
  var authFunc = function(xhr) {
    xhr.setRequestHeader("Authorization",
        "Basic " + base64.encode(username + ":" + password));
  };

  $.ajax({
    url : url,
    data : {action : "lock", force : (force ? true : undefined)},
    beforeSend : authFunc
  }).done(function (data) {
    mctx.key = data.key;
    if (data.status < 0) {
      alert("no - " + data.description);
    } else {
      mctx.has_control = true;
      alert("yes - " + mctx.key);
    }
  }).fail(function (jqXHR) {
    mctx.key = undefined;
    mctx.has_control = false;
    alert("no");
  });
};

$.fn.setErrorLog = function () {
  var url = mctx.api + "errorlog";
  var outdiv = this;
  
  var updater = function () {
    $.ajax({url : url}).done(function (data) {
      outdiv.text(data);
      setTimeout(updater, 1000);
    }).fail(function (jqXHR) {
      outdiv.text("Failed to retrieve the error log.");
    });
  };
  
  updater();
};