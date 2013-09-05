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
ut.ckey = undefined;
ut.controlcb = $.Callbacks();

/**
 * Sends an AJAX query to the API
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
    async: opts.async
  }).done(opts.callback)
    .fail(function(jqXHR) {
      ok(false, "Request failed: " + jqXHR.status.toString() + " " + jqXHR.statusText);
      opts.callback(null);
    });
}

QUnit.module("API basics");
QUnit.asyncTest("API Existence (identify)", function () {
  query("identify", {callback : function(data) {
   start();
   ok(data.status > 0, "Return status");
   ok(data.description, data.description);
   ok(data.build_date, data.build_date);
  }});
});

QUnit.asyncTest("Invalid module", function () {
  query("dontexist", {callback : function(data) {
   start();
   ok(data.status < 0);
  }});
});

QUnit.module("Sensors");
QUnit.asyncTest("Existence", function() {
  query("sensors", {params : {id : 0}, callback : function(data) {
   start();
   ok(data.status > 0, "Return status");
   ok(data.data !== undefined, "Data field existence");
   var result = "Data: ";
   for (var i = 0; i < data.data.length; i++) {
     result += data.data[i][0]  + ":" + data.data[i][1] + ", ";
   }
   ok(true, result);
  }});  
});

QUnit.asyncTest("Invalid sensor id 1", function() {
  query("sensors", {params : {id : 999}, callback : function(data) {
   start();
   ok(data.status < 0, "Return status");
  }});  
});

QUnit.asyncTest("Invalid sensor id 2", function() {
  query("sensors", {params : {id : ""}, callback : function(data) {
   start();
   ok(data.status < 0, "Return status");
  }});  
});

QUnit.asyncTest("Out of bounds sensor id 1", function() {
  query("sensors", {params : {id : "-1"}, callback : function(data) {
   start();
   ok(data.status < 0, "Return status");
  }});  
});

QUnit.asyncTest("Out of bounds sensor id 2", function() {
  query("sensors", {params : {id : "999"}, callback : function(data) {
   start();
   ok(data.status < 0, "Return status");
  }});  
});

QUnit.module("Controls and access");
QUnit.asyncTest("Gaining access", function() {
  ut.controlcb.add(function () {
    query("control", {params : {action : "start", force : true}, 
                    username : $("#username").val(), password : $("#password").val(),
                    async : false, 
                    callback : function(data) {
     start();
     ok(data.status > 0, "Return status");
     ut.ckey = data.key;
    }});
  });
});

QUnit.asyncTest("Setting actuator value", function () {
  ut.controlcb.add(function () {
    query("control", {params : {action : "set", id : 0,
          username : $("#username").val(), password : $("#password").val(),
          value : 200, key : ut.ckey},
        callback : function(data) {
          start();
          ok(data.status > 0, "Return status");
          ok(true, data.description);
    }});
  });
});

$(document).ready(function(){
  $("#control").submit(function () {
    ut.controlcb.fire();
    return false;
  });
});