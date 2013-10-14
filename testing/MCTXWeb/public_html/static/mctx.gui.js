/**
* MCTX3420 2013 GUI stuff.
* Coding style:
*  - Always end statements with semicolons
*  - Egyptian brackets are highly recommended (*cough*).
*  - Don't use synchronous stuff - hook events into callbacks
*  - $.fn functions should return either themselves or some useful object
*    to allow for chaining of method calls
*/

mctx = {};
//Don't use this in the final version
mctx.location = window.location.pathname;
mctx.location = mctx.location.substring(0, mctx.location.lastIndexOf('/')) + "/";
//mctx.location = location.protocol + "//" + location.host + "/";
mctx.api = location.protocol + "//" + location.host + "/" + "api/";
mctx.expected_api_version = 0;
mctx.has_control = false;
mctx.debug = true;

mctx.menu = [
    {'text' : 'Home', href : mctx.location + 'index.html'},
    {'text' : 'Experiment control', href : mctx.location + 'control.html'},
    {'text' : 'Pin debugging', href : mctx.location + 'pintest.html'},
    {'text' : 'Help', href : mctx.location + 'help.html'}
];

mctx.statusCodes = {
    STATUS_OK : 1
};

mctx.statusCodesDescription = {
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
    5 : {name : "Pressure sensor 2"},
    6 : {name : "Pressure sensor 3"}
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
* Logs a message if mctx.debug is enabled. This function takes
* a variable number of arguments and passes them 
* to alert or console.log (based on browser support).
* @returns {undefined}
*/
function debugLog () {
    if (mctx.debug) {
        if (typeof console === "undefined" || typeof console.log === "undefined") {
            for (var i = 0; i < arguments.length; i++) {
                alert(arguments[i]);
            }
        } else {
            console.log.apply(this, arguments);
        }
    }
}

/**
* Writes the current date to wherever it's called.
*/
function getDate() {
    document.write((new Date()).toDateString());
}

/**
* Should be run before the load of any GUI page.
* To hook events to be called after this function runs,
* use the 'always' method, e.g runBeforeLoad().always(function() {my stuff});
* @param {type} isLoginPage
* @returns The return value of calling $.ajax
*/
function runBeforeLoad(isLoginPage) {
    return $.ajax({
        url : mctx.api + "identify"
    }).done(function (data) {
        if (data.logged_in && isLoginPage) {
            if (mctx.debug) {
                debugLog("Redirect disabled!");
            } else {
                window.location = mctx.location;
            }
        } else if (!data.logged_in && !isLoginPage) {
            if (mctx.debug) {
                debugLog("Redirect disabled!");
            } else {
                //Note: this only clears the nameless cookie
                document.cookie = ""; 
                window.location = mctx.location + "login.html";
            }
        } else {
            mctx.friendlyName = data.friendly_name;
        }
    }).fail(function (jqHXR) {
        if (mctx.debug) {
            debugLog("Failed to ident server. Is API running?")
        } else if (!isLoginPage) {
            window.location = mctx.location + "login.html";
        }
    }).always(function () {
        
    });
}

/**
 * Populates the navigation menu.
 */
$.fn.populateNavMenu = function() {
    var root = $("<ul/>")
    for (var i = 0; i < mctx.menu.length; i++) {
        var item = mctx.menu[i];
        var entry = $("<li/>").append(
            $("<a/>", {text : item.text, href: item.href})
        );
        root.append(entry);
    }
    $(this).append(root);
    return this;
}

/**
* Sets the camera autoupdater
* Obsolete?
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
        
        setTimeout(updater, 10000);
    };

    updater();
    return this;
};

/**
* Sets the strain graphs to graph stuff. Obsolete?
* @returns {$.fn}
*/
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
            setTimeout(updater, 1000);
        }, function () {debugLog("It crashed");});
    };

    updater();
    return this;
};

/**
* Performs a login attempt.
* @returns The AJAX object of the login request */
$.fn.login = function () {
    var username = this.find("input[name='username']").val();
    var password = this.find("input[name='pass']").val();
    var out = this.find("#result");
    var redirect = function () {
        window.location.href = mctx.location;
    };

    out.removeAttr("class");
    out.text("Logging in...");

    return $.ajax({
        url : mctx.api + "bind",
        data : {user: username, pass : password}
    }).done(function (data) {
        if (data.status < 0) {
            mctx.has_control = false;
            out.attr("class", "fail");
            out.text("Login failed: " + data.description);
        } else {
            //todo: error check
            mctx.has_control = true;
            out.attr("class", "pass");
            out.text("Login ok!");
            setTimeout(redirect, 800);      
        }
    }).fail(function (jqXHR) {
        mctx.has_control = false;
        out.attr("class", "fail");
        out.text("Login request failed - connection issues.")
    });
};

/**
* Performs a logout request. The nameless cookie is
* always cleared and the browser redirected to the login page,
* independent of whether or not logout succeeded.
* @returns  The AJAX object of the logout request.
*/
$.fn.logout = function () {
    return $.ajax({
        url : mctx.api + "unbind"
    }).always(function () {
        //Note: this only clears the nameless cookie
        document.cookie = ""; 
        window.location = mctx.location + "login.html";
    });
};

/**
* Sets the error log to continuously update.
* @returns itself */
$.fn.setErrorLog = function () {
    var url = mctx.api + "errorlog";
    var outdiv = this;

    var updater = function () {
        $.ajax({url : url}).done(function (data) {
            outdiv.text(data);
            outdiv.scrollTop(
            outdiv[0].scrollHeight - outdiv.height()
            );
            setTimeout(updater, 3000);
        }).fail(function (jqXHR) {
            if (jqXHR.status === 502 || jqXHR.status === 0) {
                outdiv.text("Failed to retrieve the error log.");
            }
            setTimeout(updater, 10000); //poll at slower rate
        });
    };

    updater();
    return this;
};

$(document).ajaxError(function (event, jqXHR) {
    //console.log("AJAX query failed with: " + jqXHR.status + " (" + jqXHR.statusText + ")");
});