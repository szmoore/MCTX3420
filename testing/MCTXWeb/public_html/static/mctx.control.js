/**
 * Code for the controls page.
 * @date 19-10-2013
 */

mctx.control = {};
mctx.control.api = mctx.api + 'control'
mctx.control.states = {
  start : 0,
  pause : 1,
  resume : 2,
  stop : 3,
  emergency : 4
};
mctx.control.state = null;

function toggleControls(running) {
  if (running) {
    $("#experiment-stop").show();
    $("#pressure-widget").show();
    $("#start-widget").hide();
  } else {
    $("#start-widget").show();
    $("#experiment-stop").hide();
    $("#pressure-widget").hide();
  }
}

$.fn.setStatusUpdater = function () {
  var result = this;
  
  var updater = function () {
    $.ajax({
      url : mctx.control.api,
      data : {'action' : 'identify'}
    }).done(function (data) {
      if (!result.checkStatus(data)) {
        $(result).parent().addClass("fail");
        setTimeout(updater, 4000);
        return;
      }

      var text;
      var running = false;
      var fail = false;
      switch (data.control_state_id) {
        case mctx.control.states.start:
          text = "Experiment started - '" + data.control_experiment_name +
                 "' by " + data.control_user_name;
          running = true;
        break;
        case mctx.control.states.pause:
          text = "Experiment paused - '" + data.control_experiment_name +
                 "' by " + data.control_user_name;
          running = true;
        break;
        case mctx.control.states.stop:
          text = "No experiment running.";
        break;
        case mctx.control.states.emergency:
          text = "Emergency mode - '" + data.control_experiment_name +
                 "' by " + data.control_user_name;
          running = true;
          fail = true;
        default:
          text = "Unknown mode: " + data.control_state_id;
          fail = true;
      }
      
      if (data.control_state_id !== mctx.control.state) {      
        toggleControls(running);
        $(result).text(text);
        if (fail) {
          $(result).parent().addClass("fail");
        } else {
          $(result).parent().addClass("pass");
        }
        
        mctx.control.state = data.control_state_id;
      }
      
      setTimeout(updater, 2000);
    })
   .fail(function () {
     $(result).text("Connection failed.").parent().addClass("fail");
     setTimeout(updater, 4000);
   });
  };
  
  updater();
};


$.fn.startExperiment = function (group, experiment, force, result) {
 $(group).attr('disabled', 'disabled');
 
 if (!experiment || !experiment.match(/^[a-zA-Z0-9_-]+$/)) {
   result.text("Experiment names must be composed of alphanumeric characters" + 
               " or the characters -_-").addClass("fail");
   $(group).removeAttr('disabled');
   return;
 } 
 
 var data = {action : "start", name : experiment};
 if (force) {
   data.force = 1;
 }
 
 $.ajax({
   url : mctx.control.api,
   data : data
 }).done(function (data) {
   if (!result.checkStatus(data)) {
     return;
   }
   result.html("&nbsp;");
   toggleControls(true);
 }).always(function () {
   $(group).removeAttr('disabled');
 });
};

$.fn.stopExperiment = function (result) {
  var stop = this;
  stop.attr('disabled', 'disabled');
  result.text("Stopping the experiment...");
  
  $.ajax({
    url : mctx.control.api,
    data : {action : "stop"}
  }).done(function (data) {
    if (!result.checkStatus(data)) {
      return;
    }
    result.html("&nbsp;");
    toggleControls(false);
  }).always(function () {
    stop.removeAttr('disabled');
  });
};

$.fn.setPressure = function(pressure, result) {
  result.html("&nbsp;");
  
  for (var k in pressure) {
    var n = Number(pressure[k]);
    if (isNaN(n) || n < 0) {
      result.text("You must give positive numeric values.").addClass("fail");
      return;
    }
    pressure[k] = n;
  }
  
  var set = pressure['set'] + "," + pressure['wait'] + ","
            pressure['size'] + "," + pressure['count'];
  $.ajax({
    url : mctx.api + "actuators",
    data : {id : mctx.actuator.pressure_regulator, set : set}
  }).done(function (data) {
    if (!result.checkStatus(data)) {
      return;
    }
    
    result.text("Set ok!").removeClass("fail").addClass("pass");
  });
};