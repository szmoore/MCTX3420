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
mctx.graph.running = false;
mctx.graph.chart = null;

/**
 * Helper - Calculate pairs of (dependent, independent) values
 * Given input as (time, value) pairs for dependent and independent
 * Appends each value pair to the result
 * @returns result
 */
/**
 * Helper - Calculate pairs of (dependent, independent) values
 * Given input as (time, value) pairs for dependent and independent
 * Appends each value pair to the result
 * @param {array[][]} dependent Dependent data to be correlated with independent
 * @param {array[][]} independent Independent data
 * @param {array[][]} result Storage location
 * @returns {dataMerge.result}
 */
function dataMerge(dependent, independent, result) {
	var j = 0;
	for (var i = 0; i < dependent.length-1; ++i) {
		var start = dependent[i][0];
		var end = dependent[i+1][0];
		var average = 0, n = 0;
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
 * @param input_type is it a radio? or is it a checkbox?
 * @param check_first determines whether the first item is checked or not
 * @param group which group this input belongs to (name field)
 */
$.fn.deployDevices = function(input_type, check_first, group) {
  var container = this;
  var apply = function(dict, prefix) {
    $.each(dict, function(key, val) {
      var attributes = {
          'type' : input_type, 'value' : key, 'alt' : val,
          'class' : prefix, 'name' : group, 
          'id' : prefix + '_' + val //Unique id (name mangling)
      };
      var entry = $("<input/>", attributes);
      var label = $("<label/>", {'for' : prefix + '_' + val, 'text' : val}); 
      entry.prop("checked", check_first);
      check_first = false;
      container.append(entry).append(label);
    });
  }
  
  apply(mctx.sensors, 'sensors');
  apply(mctx.actuators, 'actuators');
};

/**
 * Identify sensors/actuators
 * @returns itself (Is this right?)
 */
$.fn.setDevices = function() {
  // Query for sensors and actuators
  return $.ajax({
    url : mctx.api + 'identify', 
    data : {'sensors' : 1, 'actuators' : 1}
  }).done(function (data) {
    mctx.sensors = $.extend(mctx.sensors, data.sensors);
    mctx.actuators = $.extend(mctx.actuators, data.actuators);
    
    //Always set the 'time' option to be checked
    $("#xaxis input").prop('checked', true);  
    $("#xaxis").deployDevices("radio", false, 'xaxis');
    $("#yaxis").deployDevices("checkbox", true, 'yaxis');
    $("#current_time").val(data.running_time);
    //Add event listeners for when the
    $(".change input").change(function () {
      $("#graph").setGraph();
    });
  });
};

function setGraphStatus(on, failText) {
  if (on) {
    mctx.graph.running = true;
    $("#status-text").html("&nbsp;");
    $("#graph-run").text("Pause");
  } else {
    mctx.graph.running = false;
    if (failText) {
      $("#status-text").text(failText).addClass("fail");
    } else {
      $("#status-text").text("Graph stopped").removeClass("fail");
    }
    $("#graph-run").text("Run");
  }
}

function graphUpdater() {
  var urls = {
    'sensors' : mctx.graph.api.sensors,
    'actuators' : mctx.graph.api.actuators
  }
  
  var updater = function () {
    var responses = [];
    var ctime =  $("#current_time");
    
    var xaxis = mctx.graph.xaxis;
    var yaxis = mctx.graph.yaxis;
    var start_time = mctx.graph.start_time;
    var end_time = mctx.graph.end_time;
    var devices = mctx.graph.devices;
    
    if (xaxis.size() < 1 || yaxis.size() < 1) {
      setGraphStatus(false, "No x or y axis selected.");
      return;
    }
    
    $.each(devices, function(key, val) {
      if (val.urltype in urls) {
        var parameters = {id : val.id};
        if (start_time !== null) {
          parameters.start_time = start_time;
        }
        if (end_time !== null) {
          parameters.end_time = end_time;
        }
        responses.push($.ajax({url : urls[val.urltype], data : parameters})
        .done(function(json) {
          //alert("Hi from " + json.name);
          var dev = val.data;
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
        if (xaxis.attr("alt") === "time") {
          //alert("Against time");
          plot_data.push(devices[$(this).attr("alt")].data);
        } else {
          var result = []
          dataMerge(devices[xaxis.attr("alt")].data, 
                    devices[$(this).attr("alt")].data, result);
          plot_data.push(result);
        }
      });
      
      //$.plot("#graph", plot_data);
      if (mctx.graph.chart !== null) {
        mctx.graph.chart.setData(plot_data);
        mctx.graph.chart.setupGrid(); 
        mctx.graph.chart.draw();
      } else {
        mctx.graph.chart = $.plot("#graph", plot_data);
      }
      if (mctx.graph.running) {
        mctx.graph.timer = setTimeout(updater, 1000);
      }
    }, function () {
      setGraphStatus("Connection issue - graph stopped.");
      //This will always happen when a user closes the page
      //alert("Graph crashed"); 
    });
  };
  
  setGraphStatus(true);
  updater();
  return this;
}

/**
 * Sets the graphs to graph stuff.
 * @returns {$.fn}
 */
$.fn.setGraph = function () {
  // Determine which actuator/sensors to plot
  var xaxis = $("#xaxis input[name=xaxis]:checked");
  var yaxis = $("#yaxis input[name=yaxis]:checked");
  if (xaxis.size() < 1 || yaxis.size() < 1) {
    //nothing to plot...
    setGraphStatus(false, "No x or y axis selected.");
    return;
  }
  
  var start_time = $("#start_time").val();
  var end_time = $("#end_time").val();
  if (!$.isNumeric(start_time)) {
    start_time = null;
  }
  if (!$.isNumeric(end_time)) {
    end_time = null;
  }

  var devices = {};
  var populateDict = function () {
    var dict = {};
    dict['urltype'] = $(this).attr("class");
    dict['id'] = $(this).attr("value");
    dict['data'] = [];
    dict['start_time'] = start_time;
    dict['end_time'] = end_time;
    devices[$(this).attr("alt")] = dict;
  };
  xaxis.each(populateDict);
  yaxis.each(populateDict);
  
  mctx.graph.xaxis = xaxis;
  mctx.graph.yaxis = yaxis;
  mctx.graph.start_time = start_time;
  mctx.graph.end_time = end_time;
  mctx.graph.devices = devices;
  
  if (!mctx.graph.running) {
    $("#graph-run").val("Pause");
    $("#status-text").text("")
    graphUpdater();
  }
  
  return this;
};

$.fn.runButton = function() {
  if (mctx.graph.running) {
    setGraphStatus(false);
    clearTimeout(mctx.graph.timer);
  } else {
    $("#graph").setGraph();
  }
};
