$(document).ready(function()
{
	// Send AJAX request
	$.ajax({url : "/sensor", data : {}}).done(function(data) {$(this).update(data)});

	// Interpret AJAX response
	$.fn.update = function(data)
	{
		console.log("Interpret AJAX response: "+data);
		$("#body").html("<a href=\"/index.html\">"+data+"</a>")
	}
	
});

