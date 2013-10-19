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

$.fn.initialiseControls = function () {
  var result = this;
  
  $.ajax({
    url : mctx.control.api,
    data : {'action' : 'identify'}
  }).done(function (data) {
    if (!result.checkStatus(data)) {
      $(result).parent().addClass("fail");
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
    
    if (running) {
      $("#experiment-stop").show();
      $("#pressure-widget").show();
    } else {
      $("#start-widget").show();
    }
    
    $(result).text(text);
    if (fail) {
      $(result).parent().addClass("fail");
    } else {
      $(result).parent().addClass("pass");
    }
  });
};
