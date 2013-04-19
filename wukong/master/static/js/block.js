// vim: ts=4 sw=4
var Block_count=0;
Block.linkStart = null;
Block.widgets=[];
Block.classes={};
function Block()
{
	this.init();
}
Block.register=function(type,b)
{
	Block.classes[type] = b;
}
Block.prototype.init=function() {
	this.id = Block_count;
	this.div = $('<div></div>');
	this.type='block';
	this.div.attr('id','obj'+this.id);
	this.div.css('position','absolute');
	this.div.css('cursor','pointer');
	this.div.css('background-color','#00ff00');
	this.div.attr('class','block');
	this.setPosition(250,50);
//	this.setPosition(Math.floor((Math.random()*100)),Math.floor((Math.random()*50)));
	this.setSize(100,100);
	this.location = '';
	this.group_size = 1;
	this.reaction_time = 1;
	this.signals=[];
	this.actions=[];
	this.slots=[];
	this.sigProper=[];
	this.actProper=[];
	this.monitorProper=[];
	this.numSlot = 0;
	Block_count++;
	Block.widgets.push(this);
}

Block.getBlockTypes=function() {
	var types=[];

	for(var k in Block.classes) types.push(k);
	return types;
}

Block.factory=function(type) {
	var i;
	var cls = Block.classes[type];

	if (cls)
		return new cls();
	else
		return new Block();
}

Block.prototype.serialize=function(obj) {
	obj.id = this.id;
	var pos = this.getPosition();
	var size = this.getSize();
	obj.x = pos[0];
	obj.y = pos[1];
	obj.w = size[0];
	obj.h = size[1];
	obj.type = this.type;
	obj.location = this.location;
	obj.group_size = this.group_size;
	obj.reaction_time = this.reaction_time;
	obj.actions = this.actProper;
	obj.signals = this.sigProper;
	obj.monitor = this.monitorProper;
	/*
	actlist = this.getActions();
	for(l=0;l<this.actProper.length;l++){
		obj.ac
		act = actlist[l];
		obj.actions[act.name] = this.actProper[act.name];
	}
	for(l=0;l<this.monitorProper.length;l++){
		act = actlist[l];
		obj.monitor[act.name] = this.monitorProper[act.name];
	}
	siglist = this.getSignals();
	for(l=0;l<this.sigProper.length;l++) {
		sig = siglist[l];
		obj.signals[sig.name] = this.sigProper[sig.name];
	}
	*/
	return obj;
}
Block.restore=function(a) {
	var n = Block.factory(a.type);
	n.id = a.id;
	n.setPosition(a.x,a.y);
	n.setSize(a.w,a.h);
	n.type = a.type;
	n.location = a.location;
	n.group_size = a.group_size;
	n.reaction_time = a.reaction_time;
    n.actProper = a.actProper;
    n.sigProper = a.sigProper;
    n.monitorProper = a.monitorProper;

	// Call the restore of the derived class in the future
	return n;
}
Block.prototype.draw=function() {
	var i;
	var pos = this.getPosition();
	var size = this.getSize();
	this.div.empty();
    this.div.append('<span style="font-family:"Trebuchet MS", Helvetica, sans-serif; font-size: 20pt; word-wrap: break-word;">' + this.type.replace('_', ' ') + '</span>');
	for(i=0;i<this.slots.length;i++) {
		this.div.append('<div class=signal id=signal_'+this.id+'_'+i+'>');
		$('#signal_'+this.id+'_'+i).css('position','absolute').css('width',100).css('height',15).css('left',0).css('top',i*15+20);
		$('#signal_'+this.id+'_'+i).html(this.slots[i].name.replace('_', ' '));
	}
}
Block.prototype.addSignal=function(con) {
	var i;

	for(i=0;i<this.slots.length;i++) {
		if (this.slots[i].name == con.name) {
			con.index = this.slots[i].index;
			break;
		}
	}
	if (i == this.slots.length) {
		this.slots.push(con);
		con.index = this.slots.length-1;
	}
	this.signals.push(con);
//	this.signals[con]=type;
}
Block.prototype.getSignals=function() {
	return this.signals;
}
Block.prototype.addAction=function(con) {
	var i;

	for(i=0;i<this.slots.length;i++) {
		if (this.slots[i].name == con.name) {
			con.index = this.slots[i].index;
			break;
		}
	}
	if (i == this.slots.length) {
		this.slots.push(con);
		con.index = this.slots.length-1;
	}
	this.actions.push(con);
//	this.actions[con]=type;
}
Block.prototype.setProperty=function(property,value) {
}
Block.prototype.findSignalPos=function(s) {
	var i;

	for(i=0;i<this.signals.length;i++) {
		if (this.signals[i].name == s)
			return this.signals[i].index;
	}
	return -1;
}
Block.prototype.findActionPos=function(s) {
	var i;

	for(i=0;i<this.actions.length;i++) {
		if (this.actions[i].name == s)
			return this.signals[i].index;
	}
	return -1;
}
Block.prototype.getActions=function() {
	return this.actions;
}

Block.prototype.attach=function(parent) {
	parent.append(this.div);
	this.div.draggable();
	var self = this;
	this.div.click(function() {
		if (Block.current) {
			Block.current.div.resizable("destroy");
			Block.current.setFocus(false);
		}
		self.div.resizable();
		self.setFocus(true);
		Block.current = self;
	});
	this.div.bind("dragstop", function(event,ui) {
		FBP_refreshLines();
	});
	this.div.dblclick(function() {
		$('#propertyeditor').empty();
		$('#propertyeditor').append('<div id=propertyeditor_tab>');
		$('#propertyeditor_tab').append('<ul><li><a href=#propertyeditor_loc>Location</a></li><li><a href=#propertyeditor_ft>Fault Tolerance</a></li><li><a href=#propertyeditor_action>Actions</a></li><li><a href=#propertyeditor_signal>Signals</a></li><li><a href=#propertyeditor_monitor>Monitors</a></li></ul>');

		$('#propertyeditor_tab').append('<div id=propertyeditor_loc><input type=text id=propertyeditor_location></input></div>');

		$('#propertyeditor_tab').append('<div id=propertyeditor_ft><label for="propertyeditor_groupsize">Group Size</label>');
		$('#propertyeditor_ft').append('<br><input id=propertyeditor_groupsize name=value></input>');
		$('#propertyeditor_ft').append('<br>');
		$('#propertyeditor_ft').append('<label for="propertyeditor_reactiontime">Reaction Time</label>');
		$('#propertyeditor_ft').append('<br><input id=propertyeditor_reactiontime name=value></input></div>');
		$('#propertyeditor_ft').append('');

		$('#propertyeditor_location').val(self.location);
		$('#propertyeditor_groupsize').spinner();
		$('#propertyeditor_groupsize').spinner("value",self.group_size);
		$('#propertyeditor_reactiontime').spinner();
		$('#propertyeditor_reactiontime').spinner("value",self.reaction_time);

 
		$("#propertyeditor_tab").append('<div id=propertyeditor_action></div>');
		$("#propertyeditor_tab").append('<div id=propertyeditor_signal></div></div>');
		$("#propertyeditor_tab").append('<div id=propertyeditor_monitor></div></div>');
		$("#propertyeditor_action").empty();
		$("#propertyeditor_signal").empty();
		$("#propertyeditor_monitor").empty();
		$("#propertyeditor_tab").tabs();
		
		var _actlist = Block.current.getActions();
		for(i=0;i<_actlist.length;i++) {
    		var act = _actlist[i];
    		$('#propertyeditor_action').append(act.name);
//    		if(act.type=="boolean"){
	    		$('#propertyeditor_action').append('<input type=text id=a'+act.name+'></input><br>');
	    		$('#a'+act.name).val(self.actProper[act.name]);
//	    	}else{
//	    		$('#propertyeditor_action').append('<select id=a'+act.name+'></select><br>');
//    			for(j=0;j++;j<2){
//	    			$('#propertyeditor_action').append('<option value='+j+'>'+j+'</option>');
//	    		}
//	    		$('#a'+act.name).val(self.actProper[i]);
//	    	}
			$('#propertyeditor_monitor').append(act.name+'<input type=checkbox id=m'+act.name+'></input><br>');
			try {
				$('#m'+act.name).val(self.monitorProper[act.name]);
			} catch (e){

			}

		}
		var _siglist = Block.current.getSignals();
		for(i=0;i<_siglist.length;i++) {
    		var sig = _siglist[i];
    		$('#propertyeditor_signal').append(sig.name);
    		$('#propertyeditor_signal').append('<input type=text id=s'+sig.name+'></input><br>');
    		try {
    			$('#s'+sig.name).val(self.sigProper[sig.name]);
    		} catch (e) {

    		}
    		
		}

		$('#propertyeditor').dialog({
			buttons: {
				'OK': function () {
					self.location = $('#propertyeditor_location').val();
					self.group_size = $('#propertyeditor_groupsize').spinner("value");
					self.reaction_time = $('#propertyeditor_reactiontime').spinner("value");
					for(i=0;i<_siglist.length;i++){
						sig = _siglist[i];
						self.sigProper[sig.name]=$('#s'+sig.name).val();
					}
					for(i=0;i<_actlist.length;i++){
						act = _actlist[i];
						self.actProper[act.name]=$('#a'+act.name).val();
						self.monitorProper[act.name]=$('#m'+act.name).val();
					}
					$('#propertyeditor').dialog("close");
				},
				'Cancel': function() {
					$('#propertyeditor').dialog("close");
				}
			},
			width:700, height:400,
			title:"Property Editor"

		}).dialog("open");
	});
	this.draw();
}

Block.prototype.enableContextMenu=function(b) {
	if (b) {
		var self = this;
		$('#myMenu').css('left',this.div.css('left'));
		$('#myMenu').css('top',this.div.css('top'));
		$('#myMenu').menu({
			menu:'myMenu'
		}, function(action, el, pos) {
			if (action == 'link') {
				if (Block.linkStart != null) {
					if (Block.linkStart == self) {
						alert("Can not link to myself");
						return;
					}
					new Link(Block.linkStart,"on",self,"on");
					Block.linkStart.setFocus(false);
					Block.linkStart = null;
				} else {
					Block.linkStart = self;
					self.setFocus();
				}
			}
		});
	} else {
		$('#myMenu').hide();
	}
}

Block.prototype.setFocus=function(b) {
	if (b) {
		this.div.addClass('shadow');
		this.enableContextMenu(true);
	} else {
		this.div.removeClass('shadow');
		this.enableContextMenu(false);
	}
}

Block.prototype.setPosition=function(x,y) {
	this.div.css('left',x).css('top',y);
}

Block.prototype.setSize=function(w,h) {
	this.div.css('width',w).css('height',h);
}
Block.prototype.getDIV=function() {
	return this.div;
}

Block.prototype.getPosition=function() {
	return [Block.getpx(this.div.css('left')),Block.getpx(this.div.css('top'))];
}
Block.prototype.getSize=function() {
	return [Block.getpx(this.div.css('width')),Block.getpx(this.div.css('height'))];
}


Block.prototype.refresh=function(w) {
	this.setPosition(w['x'],w['y']);
}


Block.getClass=function(name) {
	return Block.classes[name];
}

Block.getpx=function(v) {
	var index = v.indexOf('px');
	if (index >= 0) 
		return parseInt(v.substr(0,index));
	else
		return parseInt(v);
}

Block.prototype.getBounds=function() {
	var pos = this.getPosition();
	var size = this.getSize();
	var bounds = {};
	bounds.left = pos[0];
	bounds.top = pos[1];
	bounds.right = pos[0]+size[0];
	bounds.bottom = pos[1]+size[1];
	return bounds;
}

Block.hitTest=function(x,y) {
	var i;

	for(i=0;i<Block.widgets.length;i++) {
		var bounds = Block.widgets[i].getBounds();
		if(x >= bounds.left){
			if(x <= bounds.right){
				if(y >= bounds.top){
					if(y <= bounds.bottom){
						return Block.widgets[i];
					}
				}
			}
		}
	}
	return null;
}


$(document).ready(function() {
	$('#propertyeditor').dialog({autoOpen:false});
});
