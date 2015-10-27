// vim: ts=4 sw=4
function Jump(line)
{
	var ta = document.getElementById("console");

}
var g_cont = '';
$(document).ready(function() {
		var ws;
        ws = new WebSocket("ws://127.0.0.1:8888/ws");
        $(ws).bind('open', function () {
			ws.send(JSON.stringify({'cmd':'get','objects':[{'type':'image','id':'mypic'}]}));
        });
        $(ws).bind('message', function (e) {
			$('#mypic').attr('src',e.originalEvent.data+'.jpg');
			$('#uid').text(e.originalEvent.data);
		});
		$('#next').click(function() {
			ws.send(JSON.stringify({'cmd':'next'}));
		});
});
