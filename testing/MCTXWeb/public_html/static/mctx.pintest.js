/**
 * mctx.pintest: Pin test stuff.
 * Must be included after mctx.gui.js
 */


mctx.pintest = {};
mctx.pintest.gpios = [	  
  4,   5,   8,   9,  10,  11,  14,  15,  26,  27,  30,  31,  44,  45,
  46,  47,  48,  49,  60,  61,  65,  66,  67,  68,  69,  70,  71,  72,
  73,  74,  75,  76,  77,  78,  79,  80,  81,  86,  87,  88,  89, 112, 115
 ];
mctx.pintest.pwms = [0, 1, 2, 3, 4, 5, 6, 7];
mctx.pintest.refreshRate = 750;


function unexport (type, number, container, menu) {
  var url = mctx.api + "pin";

  $.ajax({url : url, data : {type : type, num : number, export : -1}})
  .fail(function () {
    switch(type) {
      case "adc" :
        var text = container.find("input[type='text'][name='" + number + "']");
        var check = container.find("input[type='checkbox'][name='unexport']");
        text.empty();
        check.attr("checked", true);
        break;
      case "gpi": case "gpo" :
          container.remove();
          menu.append($("<option />").val(number).text("GPIO " + number));
        break;
      case "pwm":
          container.remove();
          menu.append($("<option />").val(number).text("PWM " + number));
        break;
    }
  })
  .done(function () {
    
  });
}

$.fn.populateDropdown = function(items, pretext) {
  var options = this;
  $.each(items, function(index, value) {
    options.append($("<option />").val(value).text(pretext + value));
  });
};

$.fn.exportGPIO = function(menu) {
  var number = menu.val();
  var url = mctx.api + "pin";
  var container = this;
  
  $.ajax({url : url, data : {type : "gpi", num : number, export : 1}})
  .fail(function () {
    var form = $("<form/>", {"class" : "controls", action : "#", id : "gpio-" + number});
    var title = $("<div/>", {"class" : "centre bold", text : "GPIO " + number});
    var table = $("<table/>", {"class" : "centre"});
    var header = $("<tr/>");
    var controls = $("<tr/>");
    
    header.append($("<th/>", {text : "Direction"}))
      .append($("<th/>", {text : "Set"}))
      .append($("<th/>", {text : "Result"}))
      .append($("<th/>", {text : "Unexport"}));
    
    controls.append($("<td/>").append(
              $("<input/>", {type : "button", value : "In", name : "dir"})))
      .append($("<td/>").append(
        $("<input/>", {type : "button", value : "Off", name : "set", disabled : true})))
      .append($("<td/>").append(
        $("<input/>", {type : "text", readonly : "", name : "result"})))
      .append($("<td/>").append(
        $("<input/>", {type : "checkbox", name : "unexport", value : number})));
    
    form.append(title);
    table.append(header).append(controls);
    form.append(table);
    form.setGPIOControl(number, menu);
    container.append(form);
    menu.find("option[value='" + number+"']").remove();
  })
  .done(function (jqXHR) {
    alert("Failed to export GPIO " + number + ". Is the server running?\n" +
          "Error code: " + jqXHR.status);
  });
};

$.fn.exportPWM = function(menu) {
  var number = menu.val();
  var url = mctx.api + "pin";
  var container = this;
 
  $.ajax({url : url, data : {type : "pwm", num : number, export : "1"}})
  .fail(function () {
    var form = $("<form/>", {"class" : "controls", action : "#", id : "pwm-" + number});
    var title = $("<div/>", {"class" : "centre bold", text : "PWM " + number});
    var table = $("<table/>", {"class" : "centre"});
    var header = $("<tr/>");
    var controls = $("<tr/>");
    
    header.append($("<th/>", {text : "Frequency (Hz)"}))
      .append($("<th/>", {text : "Duty cycle"}))
      .append($("<th/>", {text : "Polarity"}))
      .append($("<th/>", {text : "Set"}))
      .append($("<th/>", {text : "Result"}))
      .append($("<th/>", {text : "Unexport"}));
    
    controls.append($("<td/>").append(
              $("<input/>", {type : "text", name : "freq"})))
      .append($("<td/>").append(
        $("<input/>", {type : "text", name : "duty"})))
      .append($("<td/>").append(
        $("<input/>", {type : "checkbox", name : "pol"})))
      .append($("<td/>").append(
        $("<input/>", {type : "button", value: "Go", name : "set"})))
      .append($("<td/>").append(
        $("<input/>", {type : "text", readonly : "", name : "result"})))
      .append($("<td/>").append(
        $("<input/>", {type : "checkbox", name : "unexport", value :umber})
        .click(function () {
          unexport("pwm", number, form, menu);
          return false;
        })));
    
    form.append(title);
    table.append(header).append(controls);
    form.append(table);
    container.append(form);
    menu.find("option[value='" + number+"']").remove();
  })
  .done(function (jqXHR) {
    alert("Failed to export PWM " + number + ". Is the server running?\n" +
          "Error code: " + jqXHR.status);
  });
};

$.fn.setGPIOControl = function (number, menu) {
  var container = this;
  var dir = this.find("input[name='dir']");
  var set = this.find("input[name='set']");
  var result = this.find("input[name='result']");
  var unexport = this.find("input[name='unexport']");
  var update = true;
  var updater = function() {
    if (update) {
      $.ajax({url : mctx.api + "pin", data : {type : "gpi", num : number}})
      .done(function (data) {
        result.val(data);
      })
      .fail(function () {
        update = false;
        result.val("GPIO retrieve failed.");
      });
    }
    setTimeout(updater, mctx.pintest.refreshRate);
  };
  
  dir.click(function () {
    dir.attr('disabled', true);
    var setOut = dir.val() === "In";
    if (setOut) {
      update = false;
      set.attr('disabled', false);
      result.empty();
      dir.val("Out");
    } else {
      update = true;
      set.attr('disabled', true);
      result.empty();
      dir.val("In");
    }
    dir.attr('disabled', false);
  });
  
  set.click(function () {
    dir.attr("disabled", true);
    var val = (set.val() === "Off") ? 1 : 0;
    $.ajax({url : mctx.api + "pin", data : {type : "gpo", num : number, set : val}})
    .done(function (data) {
      result.text(data);
      if (val === 0)
        set.val("Off");
      else
        set.val("On");
    })
    .fail(function () {
      result.val("fail");
    })
    .always(function () {
      dir.attr("disabled", false);
    });
  });
  
  unexport.click(function () {
    update = false;
    $.ajax({url : mctx.api + "pin", data : {type : "gpi", num : number, export : -1}})
    container.remove();
    menu.append($("<option />").val(number).text("GPIO " + number));
    return false;
  });
  
  updater();
};

/* 
 * GPIO template
          <form class="controls" action="#">
            <div class="centre bold">GPIO 20</div>
            
            <table class="centre">
              <tr>
                <th>Direction</th><th>Set</th><th>Result</th><th>Unexport</th>
              </tr>
              <tr>
                <td><input type="button" value="Out"></td>
                <td><input type="button" value="On"></td>
                <td><input type="text" readonly></td>
                <td><input type="checkbox"></td>
              </tr>
            </table>
          </form>
 */

/*
 * PWM template
          <form class="controls" action="#">
            <table class="centre">
              <tr>
                <th>Frequency (Hz)</th><th>Duty cycle</th>
                <th>Polarity</th><th>Set</th>
                <th>Result</th><th>Unexport</th>
              </tr>
              <tr>
                <td><input type="text"></td>
                <td><input type="text"></td>
                <td><input type="checkbox"></td>
                <td><input type="button" value="Go"></td>
                <td><input type="text" readonly></td>
                <td><input type="checkbox"></td>
              </tr>
            </table>
          </form>
 */