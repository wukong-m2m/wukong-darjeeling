// vim: ts=4 sw=4
var g_selected_line = null;
function Line(source,signal,dest,action)
{
	this.source = source;
	this.signal = signal;
	this.dest = dest;
	this.action = action;
}

Line.prototype.draw=function(obj) {
	var loc = this.source.getPosition();
	var size = this.source.getSize();
	var signal_idx = this.source.findSignalPos(this.signal);
	var action_idx = this.dest.findActionPos(this.action);
	var x1 = loc[0]+size[0]-205;
	var y1 = loc[1]-FBP_CANVAS_TOP+signal_idx*15+15/2+20;
	loc = this.dest.getPosition();
	size = this.dest.getSize();
	var x2 = loc[0]-205;
	var y2 = loc[1]-FBP_CANVAS_TOP+action_idx*15+15/2+20;
	var canvas = obj[0].getContext('2d');
	canvas.save();
	var dx = x2-x1;
	var dy = y2-y1;
	var len = Math.sqrt(dx*dx+dy*dy);
	if (g_selected_line == this) {
		canvas.strokeStyle = 'red';
	} else {
		canvas.strokeStyle = 'black';
	}
	canvas.translate(x2,y2);
	canvas.rotate(Math.atan2(dy,dx));
	canvas.lineCap='round';
	canvas.beginPath();
	canvas.moveTo(0,0);
	canvas.lineTo(-len,0);
	canvas.closePath();
	canvas.stroke();
	canvas.beginPath();
	canvas.moveTo(0,0);
	canvas.lineTo(-7,-7);
	canvas.lineTo(-7,7);
	canvas.closePath();
	canvas.fill();
	canvas.restore();
}


Line.prototype.serialize=function() {
	var obj = {};
	obj.source = this.source.id;
	obj.dest = this.dest.id;
	obj.signal = this.signal;
	obj.action = this.action;
	return obj;
}

Line.restore=function(a) {
	var l = new Line(a.source,a.signal, a.dest,a.action);
	return l;
}

Line.prototype.toString=function() {
	return this.signal + ' ---> '+ this.action;
}

function Line_distance(x1,y1,x2,y2,px,py)
{
	var norm = Math.sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
	var d = Math.abs((px-x1)*(y2-y1)-(py-y1)*(x2-x1))/norm;
	return d;
}

function Line_search(lines,px,py)
{
	var i;
	var len = lines.length;
	var mind = 20;
	var l = null;
	py += FBP_CANVAS_TOP/2;

	for(i=0;i<len;i++) {
		var loc = lines[i].source.getPosition();
		var size = lines[i].source.getSize();
		var x1 = loc[0]+size[0]/2;
		var y1 = loc[1]+size[1]/2;
		loc = lines[i].dest.getPosition();
		size = lines[i].dest.getSize();
		var x2 = loc[0]+size[0]/2;
		var y2 = loc[1]+size[1]/2;
		var signal_idx = lines[i].source.findSignalPos(lines[i].signal);
		var action_idx = lines[i].dest.findActionPos(lines[i].action);
		y1 += signal_idx*15;
		y2 += action_idx*15;
		var d = Line_distance(x1,y1,x2,y2,px,py);
		console.log('x1='+x1+' y1='+y1+' x2='+x2+' y2='+y2+' d='+d);
		
		if (d < mind) {
			l=lines[i];
			mind = d;
		}
	}
	return l;
}
