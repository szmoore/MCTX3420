/**
 * @file gui.js
 * @purpose GUI in JavaScript using jQuery and jQuery Flot libraries
 * NEEDS WORK !
 */

$(document).ready(function()
{

	g_sensors = []
	g_numSensors = 1
	g_storeTime = []
	g_key = null
	g_led = 1
	
	$.fn.ledFlash = function()
	{
		//alert("Flash LED");
		if (g_led == 0)
		{
			$.ajax({url : "api/actuators", data : {id : 0, set : 1}})
			g_led = 1
			$("#led").html("LED On");
		}
		else
		{
			$.ajax({url : "api/actuators", data : {id : 0, set : 0}})
			g_led = 0
			$("#led").html("LED Off");
		}
	}

	$.fn.pruneSensorData = function(id)
	{
		var sensor = g_sensors[id]
		// remove unwanted values
		if (g_storeTime[id] != null)
		{
			var last_time = sensor[sensor.length-1][0]
			//console.log("Last time: " + String(last_time))
			var back_index = 0;
			for (back_index = sensor.length-1; back_index > 0; --back_index)
			{
				if ((last_time - sensor[back_index][0]) > g_storeTime[id])
				{
					break;
				}

				//console.log("Keep point " + String(sensor[back_index][0]) + "s ago")

			}
			if (back_index > 0)
			{
				//console.log("Delete " +String(back_index))
				sensor.splice(0, back_index)
			}
		}
	}
	

	// update function
	$.fn.updateSensor = function(json)
	{
		//console.log(json.data)
		var sensor = g_sensors[0]
		var most_recent = null
		if (sensor.length > 0)
			most_recent = sensor[sensor.length-1][0]

		for (var i=0; i < json.data.length; ++i)
		{
			if (json.data[i][0] > most_recent || most_recent == null)
			{
				sensor.push(json.data[i])
				most_recent = json.data[i][0]
			}
		}

		$(this).pruneSensorData(json.id)
		
		//console.log("Plot:")
		//console.log(g_sensors[json.id])
		$.plot("#sensor"+String(0)+"_plot", [g_sensors[0]])
		$.ajax({url : "/api/sensors", data : {id : json.id}, success : function(data) {$(this).updateSensor(data);}});
		
		//
	}

	$.fn.reloadSensor = function(id)
	{
		g_storeTime[id] = Number($("#sensor"+String(id)+"_settings").find("#storeTime").val())
		console.log(String($("#sensor"+String(id)+"_settings").find("#storeTime").val()))
		
		
		
		$(this).pruneSensorData(id)
		$.plot("#sensor"+String(id)+"_plot", [g_sensors[id]])
	}

	$.fn.setPressure = function()
	{
		var value = Number($("#control0_value").val())
		$.ajax({url : "/api/control", data : {action : "set", value : String(value), id : 0, key : g_key}, success : function(json) {alert(json.description)}})
	}

	$.fn.LoadGUI = function()
	{
		//TODO: Get rid of g_numSensors; query the server for sensors?

		$.ajax({url : "/api/control", data : {action : "start", force : "true"}, success : function(json) {g_key = String(json.key)}, async : false})
		console.log("Key is " + g_key)

		// Load the plots
		plotsHTML = ""
		for (var i = 0; i < g_numSensors; ++i)
		{
			g_sensors.push([new Array()])
			g_storeTime.push(600)
			//plotsHTML += "<div id=sensor"+String(i)+"\n"
			plotsHTML += "<h2 id=sensor"+String(i)+"_name>Sensor: "+String(i)+"</h2>\n"
			plotsHTML += "<div id=sensor"+String(i)+"_plot class=\"plot\" onclick=\"window.open('/api/sensors?id="+String(i)+"&dump')\"> </div>\n"
			plotsHTML += "<div id=sensor"+String(i)+"_settings\n"
			plotsHTML += "<p> Plot since <input type=text id=storeTime value=\""+String(g_storeTime[i])+"\"/> seconds ago\n"
			plotsHTML += "<button id=update onclick=$(document).reloadSensor("+String(i)+")>Update</button></p>\n"
			plotsHTML += "<button id=dump onclick=\"window.open('/api/sensors?id="+String(i)+"&dump')\">Dump All Data\n"
			plotsHTML += "</div>\n" // end settings
			//plotsHTML += "</div>\n" // 
			
		}
		$("#plots").html(plotsHTML)

		controlHTML = "<h2 id=control0>Controls</h2>\n"
		controlHTML += "<button id=led onclick=\"$(document).ledFlash()\">LED On</button>"
		
		$("#controls").html(controlHTML)

	}


	
	$(this).LoadGUI()

	

	for (var i = 0; i < g_numSensors; ++i)
	{
	  $.ajax({url : "/api/sensors", data : {id : 2}, success : function(data) {$(this).updateSensor(data);}})

	}
});

