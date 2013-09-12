/**
 * MCTX3420 2013 - Remote pressurised can experiment.
 * Unit testing for the server API.
 * These unit tests use the QUnit unit testing framework.
 * @requires QUnit, jQuery, and base64.js
 * @date 28/8/13
 * @author Jeremy Tan
 */

//Namespace ut

ut = {};
ut.api = location.protocol + "//" +  location.host + "/api/";
ut.controlcb = $.Deferred();

/**
 * Sends a synchronous AJAX query to the API
 * A synchronous request makes it easier for unit testing.
 * @param {string} module The name of the module to be queried
 * @param {Object} opts Object holding the parameters, username, password and
 *                 callback. The parameters should be an object of key/value
 *                 pairs.
 * @returns jqXHR object (but calls callback with JSON data, or null on AJAX error)
 */
function query(module, opts) {
  var queryurl = ut.api + module;
  
  var authfunc;
  if (opts.username) {
    authfunc = function(xhr) {
      xhr.setRequestHeader("Authorization",
        "Basic " + base64.encode(opts.username + ":" + opts.password));
    };
  }
  
  return $.ajax({
    url: queryurl,
    type: 'GET',
    dataType: 'json',
    data: opts.params,
    beforeSend: authfunc,
    async: false
  }).done(opts.callback)
    .fail(function(jqXHR) {
      ok(false, "Request failed: " + jqXHR.status.toString() + " " + jqXHR.statusText);
      opts.callback(null);
    });
}

QUnit.module("API basics");
QUnit.test("API Existence (identify)", function () {
  query("identify", {params : {actuators : true, sensors : true}, 
   callback : function(data) {
    ok(data.status > 0, "Return status");
    ok(data.description !== undefined, data.description);
    ok(data.build_date !== undefined, data.build_date);
    ok(data.api_version !== undefined, "API version: " + data.api_version);
    ok(data.sensors !== undefined, "Sensors list");
    ok(data.actuators !== undefined, "Actuators list");
    
    var sl = "Sensors: ", al = "Actuators: ";
    for (var id in data.sensors) {
      sl += id + ":" + data.sensors[id] + " ";
    }
    for (var id in data.actuators) {
      al += id + ":" + data.actuators[id] + " ";
    }
    ok(sl, sl);
    ok(al, al);
   }});
});

QUnit.test("Invalid module", function () {
  query("dontexist", {callback : function(data) {
   ok(data.status < 0);
  }});
});

QUnit.module("Sensors");
QUnit.test("Existence", function() {
  query("identify", {params : {sensors : 1}, callback : function(data) {
      ok(data.status > 0, "Identification");
      var hasSensor = false;
      for (var id in data.sensors) {
        hasSensor = true;
        query("sensors", {params : {id : id}, callback : function(data) {
          ok(data.status > 0, "Sensor " + id);
          ok(data.data !== undefined, "Data field existence");
          var result = "Data: ";
          for (var i = 0; i < data.data.length; i++) {
            result += data.data[i][0]  + ":" + data.data[i][1] + ", ";
          }
          ok(true, result);
       }});        
      }
      ok(hasSensor, "Has at least one sensor");
  }});
 
});

QUnit.test("Invalid sensor ids", function() {
  query("sensors", {params : {id : ""}, callback : function(data) {
   ok(data.status < 0, "No id");
  }});  
  query("sensors", {params : {id : 999}, callback : function(data) {
   ok(data.status < 0, "Id too large");
  }});
  query("sensors", {params : {id : "-1"}, callback : function(data) {
   ok(data.status < 0, "Negative id");
  }});  
});

QUnit.module("Controls and access");
QUnit.asyncTest("Setting actuator value", function () {
  $.when(ut.controlcb).done(function () {
    start();
    var key;
    
    query("control", {params : {action : "start", force : true}, 
                    username : $("#username").val(), password : $("#password").val(),
                    async : false, 
                    callback : function(data) {
     ok(data.status > 0, "Gaining access key");
     ok(data.key, "Access key - " + data.key);
     key = data.key;
    }});    
    query("control", {params : {action : "set", id : 0,
          username : $("#username").val(), password : $("#password").val(),
          value : 200, key : key},
        callback : function(data) {
          ok(data.status > 0, "Setting actuator");
          ok(true, data.description);
    }});
  });
});

$(document).ready(function(){
  $("#control").submit(function () {
    ut.controlcb.resolve();
    return false;
  });
});