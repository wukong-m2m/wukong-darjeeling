// vim: ts=4 sw=4
var g_selected_line = null;
function Line(source,signal,dest,action)
{
    this.source = source;
    this.signal = signal;
    this.dest = dest;
    this.action = action;
    this.p0 = this.p1 = [0,0];
}

Line.prototype.draw=function(obj,options) {
    if (typeof(options)=='undefined') options = {color:'rgba(0,0,0,0.5)',width:1}

    var viewportSize = getViewportSize()
    var loc = this.source.getAbsPosition();
    var size = this.source.getSize();
    var signal_idx = this.source.findSignalPos(this.signal);
    var action_idx = this.dest.findActionPos(this.action);
    var x1 = loc[0]+(size[0]-FBP_CANVAS_LEFT);
    var topOffset = FBP_BLOCK_SIGNALSTOP ;
    var itemHeight = FBP_BLOCK_SIGNALITEM_HEIGHT;
    var y1 = loc[1]-FBP_CANVAS_TOP+(signal_idx+0.5)*itemHeight+topOffset;
    loc = this.dest.getAbsPosition();
    size = this.dest.getSize();
    var x2 = loc[0]-FBP_CANVAS_LEFT;
    var y2 = loc[1]-FBP_CANVAS_TOP+(action_idx+0.5)*itemHeight+topOffset;
    this.p0 = [x1,y1]
    this.p1 = [x2,y2]
    var canvas = obj[0].getContext('2d');
    canvas.save();
    var dx = x2-x1;
    var dy = y2-y1;
    var len = Math.sqrt(dx*dx+dy*dy);
    canvas.globalCompositeOperation = options.erase ? 'destination-out' : ''
    /*
    if (g_selected_line == this) {
        canvas.strokeStyle = 'red';
        canvas.lineWidth = 3;
        console.log('draw on '+obj.attr('id'))
    } else {
        canvas.strokeStyle = 'rgba(0,0,0,0.5)';
        canvas.lineWidth = 1;
    }
    */
    canvas.strokeStyle = options.color;
    canvas.lineWidth = options.width;
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
    canvas.lineTo(-4,-4);
    canvas.lineTo(-4,4);
    canvas.closePath();
    canvas.fillStyle = canvas.strokeStyle;
    canvas.fill();
    canvas.restore();
}

function drawLine(canvas,p0,p1){
    canvas.save();
    canvas.lineCap='round';
    canvas.strokeStyle = 'blue';
    canvas.lineWidth = 2;
    canvas.beginPath();
    canvas.moveTo(p0.x,p0.y);
    canvas.lineTo(p1.x,p1.y);
    canvas.closePath();
    canvas.stroke();
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
Line.clearPropertyEditForm = function(){
    var editForm = document.getElementById('blockPropEditForm')
    editForm.innerHTML = ''
    sidebarActiveTab('t1')
}
Line.prototype.renderPropertyEditForm = function(){
    var tags = ['<table class="form">']
    tags.push('<tr><th style="width:40px">Signal:</th><td>'+this.signal+'</td></tr>')
    tags.push('<tr><th>Action:</th><td>'+this.action+'</td></tr>')
    tags.push('</table>')
    tags.push('<button onclick="FBP_deleteLink()">Delete</button>')
    var editForm = document.getElementById('blockPropEditForm')
    sidebarActiveTab('t3')
    editForm.innerHTML = tags.join('')
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

    var sloc;
    for(i=0;i<len;i++) {
        var loc = lines[i].source.getPosition();
        /*
        sloc = loc;
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
        */

        var x1 = lines[i].p0[0]
        var y1 = lines[i].p0[1]
        var x2 = lines[i].p1[0]
        var y2 = lines[i].p1[1]
        /* HY: Test if the point is in the rectangle of this line */
        if (px < Math.min(x1,x2) || px > Math.max(x1,x2) || py < Math.min(y1,y2) || py > Math.max(y1,y2)) continue

        var d = Line_distance(x1,y1,x2,y2,px,py);
        if (d < mind) {
            l=lines[i];
            mind = d;
        }
        //drawLine(FBP_canvas[0].getContext('2d'),{x:x1,y:y1},{x:x2,y:y2})
        //console.log(i+'('+px+','+py+'):('+x1+','+y1+') to ('+x2+','+y2+') d='+d);
        //console.log(d+';'+lines[i].toString()+',['+sloc+'],px='+px+(l ? ',l='+l : ''))
    }
    return l;
}
