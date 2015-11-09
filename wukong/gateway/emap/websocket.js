// vim: ts=4 sw=4
function Jump(line)
{
	var ta = document.getElementById("console");

}
var g_cont = '';
$(document).ready(function() {
		var ws;
		var f = window.location.href.split('//');
		f = f[1];
		f = f.split('/')
	    f = f[0]
        ws = new WebSocket("ws://"+f+"/ws");
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
