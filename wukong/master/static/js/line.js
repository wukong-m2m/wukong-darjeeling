// vim: ts=4 sw=4

function Line(source,signal,dest,action)
{
	this.source = source;
	this.signal = signal;
	this.dest = dest;
	this.action = action;
}

Line.prototype.draw=function(canvas) {
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

	canvas.drawLine({
		strokeStyle: "#000",
		strokeWidth: 4,
		x1: x1, y1:y1,
		x2: x2, y2:y2
	});
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
	var mind = 10;
	var l = null;
	//py -= FBP_CANVAS_TOP;

	for(i=0;i<len;i++) {
		var loc = lines[i].source.getPosition();
		var size = lines[i].source.getSize();
		var x1 = loc[0]+size[0]/2;
		var y1 = loc[1]+size[1]/2;
		loc = lines[i].dest.getPosition();
		size = lines[i].dest.getSize();
		var x2 = loc[0]+size[0]/2;
		var y2 = loc[1]+size[1]/2;
		var d = Line_distance(x1,y1,x2,y2,px,py);
		if (d < mind) {
			l=lines[i];
		}
	}
	return l;
}
