/**
 * MCTX3420 2013 - Remote pressurised can experiment.
 * Unit testing for the server API.
 * These unit tests use the QUnit unit testing framework.
 * @requires QUnit and jQuery
 * @date 28/8/13
 * @author Jeremy Tan
 */

var api = location.protocol + "//" +  location.host + "/api/";

/**
 * Sends an AJAX query to the API
 * @param {string} module The name of the module to be queried
 * @param {Object} opts Object holding the parameters, username, password and
 *                 callback. The parameters should be an object of key/value
 *                 pairs.
 * @returns JSON data
 */
function query(module, opts) {
  function buildQuery(opts) {
    var result = "?";
    var first = true;
    
    for (key in opts) {
      if (!first) 
        result += "&";
      else 
        first = false;
      result += encodeURIComponent(key) + 
                ((opts[key] !== undefined) ? "=" + encodeURIComponent(opts[key]) : "");
    }
    return result;
  }
  
  var queryurl = api + module;
  if (opts.params)
    queryurl += buildQuery(opts.params);
  
  var authfunc;
  if (opts.username) {
    authfunc = function(xhr) {
      xhr.setRequestHeader("Authorization",
        "Basic " + btoa(opts.username + ":" + opts.password));
    };
  }
  
  $.ajax({
    url: queryurl,
    type: 'GET',
    dataType: 'json',
    beforeSend: authfunc
  }).done(opts.callback)
    .fail(function(jqXHR) {
      alert("Request Failed!");
      ok(false, "Request failed: " + jqXHR.status.toString() + " " + jqXHR.statusText);
      opts.callback(null);
    });
}

QUnit.module("API basics");
QUnit.asyncTest("Existence (identify)", function () {
  query("identify", {callback : function(data) {
   start();
   ok(data.status >= 0, "Return status");
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
   ok(data.status >= 0, "Return status");
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

QUnit.module("Controls and access");
QUnit.asyncTest("Gaining access", function() {
  query("control", {params : {action : "start", force : true}, 
                  username : "mctxadmin", password : "admin", 
                  callback : function(data) {
   start();
   ok(data.status >= 0, "Return status");
   
   var key = data.key;
   
  }});
});
