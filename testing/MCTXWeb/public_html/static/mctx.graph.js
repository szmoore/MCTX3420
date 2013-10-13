/**
 * Graph sensor and/or actuator values
 */

//TODO: Clean this file up, I bow to Jeremy's superior JavaScript knowledge


mctx.graph = {};
mctx.graph.api = {};
mctx.graph.api.sensors = mctx.api + "sensors";
mctx.graph.api.actuators = mctx.api + "actuators";
mctx.sensors = {};
mctx.actuators = {};
mctx.graph.dependent = null;
mctx.graph.independent = null;
mctx.graph.timer = null;

/**
 * Helper - Calculate pairs of (dependent, independent) values
 * Given input as (time, value) pairs for dependent and independent
 * Appends each value pair to the result
 * @returns result
 */
function dataMerge(dependent, independent, result) {
	
	var j = 0;
	for (var i = 0; i < dependent.length-1; ++i) {
		var start = dependent[i][0];
		var end = dependent[i+1][0];
		var average = 0; var n = 0;
		for (; j < independent.length; ++j) {
			if (independent[j][0] < start)
				continue;
			else if (independent[j][0] >= end)
				break;
			average += independent[j][1];
			n += 1;
		}
		if (n > 0) {
			average /= n;
			result.push([dependent[i][1], average]);
		}	
	}
	return result;
}

/**
 * Helper function adds the sensors and actuators to a form
 */
$.fn.deployDevices = function(input_type, check_first) {
  var formhtml = $(this).html();
  var formname = $(this).attr("id");
 // formhtml += "<i> Sensors </i>"
  var checked = "checked";
  if (!check_first)
    checked = "";
  $.each(mctx.sensors, function(key, val) {
    formhtml += "<input type=\""+input_type+"\" value=\""+val+"\" id=\"sensors\" name=\""+formname+"\""+checked+">" + val + "</input>";
    checked = "";
  });
 // formhtml += "<i> Actuators </i>"
  $.each(mctx.actuators, function(key, val) {
    formhtml += "<input type=\""+input_type+"\" value=\""+val+"\" id=\"actuators\" name=\""+formname+"\""+checked+">" + val + "</input>";
    checked = "";
  });
  $(this).html(formhtml);
};

/**
 * Identify sensors/actuators
 * @returns itself (Is this right?)
 */
$.fn.setDevices = function() {
	// Query for sensors and actuators
  var sensor_curtime = 0;
  var actuator_curtime = 0;
  return $.when(
  	$.ajax({url : mctx.api + "?sensors"}).done(function(data) {
  		mctx.sensors = $.extend(mctx.sensors, data.sensors);
      sensor_curtime = data.running_time;
    }),
    $.ajax({url : mctx.api + "?actuators"}).done(function(data) {
      mctx.actuators = $.extend(mctx.actuators, data.actuators);
      actuator_curtime = data.running_time;
    })
  ).then(function() {
    $("#xaxis").deployDevices("radio", false);
    $("#yaxis").deployDevices("checkbox", true);
    var c = Math.max(actuator_curtime, sensor_curtime);
    $("input[name=current_time]", "#time_range").val(c);
    
    
  });
};




/**
 * Sets the graphs to graph stuff.
 * @returns {$.fn}
 */
$.fn.setGraph = function () {
  clearTimeout(mctx.graph.timer);
  var sensor_url = mctx.api + "sensors";
  var actuator_url = mctx.api + "actuators";

  var updateData = function(json, data) {
    for (var i = 0; i < json.data.length; ++i)
      data.push(json.data[i]);
    return data;
  };
  var graphdiv = this;


  // Determine which actuator/sensors to plot
 
  var xaxis = $("input[name=xaxis]:checked", "#xaxis");
  var yaxis = $("input[name=yaxis]:checked", "#yaxis");
  var start_time = $("#start_time").val();
  var end_time = $("#end_time").val();
  if (!$.isNumeric(start_time)) {
    start_time = null;
  }
  if (!$.isNumeric(end_time)) {
    end_time = null;
  }

  var devices = {};
  xaxis.each(function() {
    devices[$(this).val()] = {};
    devices[$(this).val()]["url"] = mctx.api + $(this).attr("id");
    devices[$(this).val()]["data"] = [];
    devices[$(this).val()]["start_time"] = start_time;
    devices[$(this).val()]["end_time"] = end_time;
  });
  yaxis.each(function() {
    devices[$(this).val()] = {};
    devices[$(this).val()]["url"] = mctx.api + $(this).attr("id");
    devices[$(this).val()]["data"] = [];
    devices[$(this).val()]["start_time"] = start_time;
    devices[$(this).val()]["end_time"] = end_time;
  });

 
  var updater = function () {
    var time_limit = 20;
    var responses = [];
    var ctime =  $("#current_time");
    
    
    $.each(devices, function(key, val) {
      if (devices[key].url === sensor_url || devices[key].url === actuator_url) {
       // alert("AJAX");
        //alert(key);
        //alert(devices[key].url);
        parameters = {name : key};
        if (start_time != null) {
          //alert("start_time = " + start_time);
          parameters = $.extend(parameters, {start_time : start_time});
        }
        if (end_time != null)
          parameters = $.extend(parameters, {end_time : end_time});
        responses.push($.ajax({url : devices[key].url, data : parameters}).done(function(json) {
          //alert("Hi from " + json.name);
          var dev = devices[json.name].data;
          for (var i = 0; i < json.data.length; ++i) {
            if (dev.length <= 0 || json.data[i][0] > dev[dev.length-1][0]) {
              dev.push(json.data[i]);
            }
          }
          ctime.val(json.running_time);
          //alert(devices[json.name].data);
        }));
      }
    });

    //... When the response is received, then() will happen (I think?)
    $.when.apply(this, responses).then(function () {
      
      var plot_data = [];
      yaxis.each(function() {
        //alert("Add " + $(this).val() + " to plot");
        if (xaxis.val() === "time") {
          //alert("Against time");
          plot_data.push(devices[$(this).val()].data);
        }
        else {
          var result = []
          dataMerge(devices[xaxis.val()].data, devices[$(this).val()].data, result);
          /*
          var astr = "[";
          for (var i = 0; i < result.length; ++i)
            astr += "[" + result[i][0] + "," + result[i][1] + "]" + ",";
          astr += "]";
          alert(astr);
          */
          plot_data.push(result);
        }
      });
      
      //alert(plot_data + "");
      //alert("Plot happened");
      $.plot("#graph", plot_data);
      mctx.graph.timer = setTimeout(updater, 1000);
    }, function () {alert("Graph crashed");});
  };
  
  updater();
  return this;
};

$.fn.runButton = function() {
  //alert($(this).val());
  if ($(this).val() === "Run") {
    $("#graph").setGraph();
    $(this).val("Pause");
  }
  else {
    clearTimeout(mctx.graph.timer);
    $(this).val("Run");
  }
};
