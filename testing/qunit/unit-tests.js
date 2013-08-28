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
 * @param {Object} opts Object containing parameters to pass to module 
 * @param {function} callback Function that receives JSON data
 * @param {string} username Optional
 * @param {string} password Required if username specified
 * @returns JSON data
 */
function query(module, opts, callback, username, password) {
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
  
  $.ajax({
    url: queryurl,
    type: 'GET',
    dataType: 'json',
    beforeSend: !username ? undefined : function (xhr) { 
        xhr.setRequestHeader("Authorization", 
            "Basic " + btoa(username + ":" + password)); 
    }
  }).done(callback)
    .fail(function(jqXHR) {
      if (jqXHR.status === 400) {
        callback($.parseJSON(jqXHR.responseText));
      } else {
        callback({status:-999, 
          description: jqXHR.status.toString() + " " + jqXHR.responseText});
      }
    });
}

QUnit.test("API Existence", function () {
  stop(); //?????
  query("test", undefined, function(data) {
   equal(parseInt(data.status, 10), -1, "Nonexistent module"); //Magic numbers!
   start();
  });   

  /*query("version", undefined, function (data) {
    assert.equal(data.status, 0);
  });*/
  
});