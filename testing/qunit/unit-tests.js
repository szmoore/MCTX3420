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
 * query(module, username, password, callback);
 * query(module, callback);
 * query(module, opts, callback);
 * query(module, opts, username, password, callback);
 * @param {string} module The name of the module to be queried
 * @param {Object} opts Object containing parameters to pass to module 
 * @param {string} username Optional
 * @param {string} password Required if username specified
 * @param {function} callback Function that receives JSON data
 * @returns JSON data
 */
function query(module, opts, username, password, callback) {
  if (typeof opts === 'string') {
    callback = password;
    password = username;
    username = opts;
    opts = undefined;
  } else if (typeof opts === 'function') {
    callback = opts;
    opts = undefined;
  } else if (typeof username === 'function') {
    callback = username;
    username = undefined;
  }
  
  function buildQuery(opts) {
    var result = "?";
    var first = true;
    
    for (key in opts) {
      if (!first) 
        result += "&";
      else 
        first = false;
      result += encodeURIComponent(key) + 
                (opts.key ? "=" + encodeURIComponent(opts.key) : "");
    }
    return result;
  }
  
  var queryurl = api + module;
  if (opts)
    queryurl += buildQuery(opts);
  
  var authfunc;
  if (username) {
    authfunc = function(xhr) {
      xhr.setRequestHeader("Authorization",
        "Basic " + btoa(username + ":" + password));
    };
  }
  
  $.ajax({
    url: queryurl,
    type: 'GET',
    dataType: 'json',
    beforeSend: authfunc
  }).done(callback)
    .fail(function(jqXHR) {
      //Note:Callback must be called so the QUnit test can run.
      if (jqXHR.status !== 400) {
        callback({"status" : jqXHR.status, "description" : jqXHR.statusText});
      } else {
        try {
          callback($.parseJSON(jqXHR.responseText));
        } catch (err) {
          callback({"status" : jqXHR.status, "description" : jqXHR.statusText});
        }
      }
    });
}


QUnit.asyncTest("API Existence", function () {
  query("test", function(data) {
   start();
   //TODO:Change fastcgi error codes
   equal(parseInt(data.status, 10), -1, "Nonexistent module"); //Magic numbers!
  });
});

QUnit.asyncTest("Login test", function() {
  query("login", {"force" : true}, "mctxadmin", "admin", function(data) {
   start();
   equal(parseInt(data.status, 10), 0, "Login ok"); //Magic numbers!
  });
});

QUnit.test("Sensors module", function() {
  
});

/*QUnit.test("Login module", function () {
  
});*/

QUnit.test("Access control", function () {
  
});