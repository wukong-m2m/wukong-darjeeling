$(document).ready(function() {
	ide = new WuIDE();
});



function WuIDE()
{
    var self = this;
	$.get('/componentxml',function(r) {
		self.xml = $.parseXML(r);
		self.classes = $(self.xml).find('WuClass');
		self.types = $(self.xml).find('WuTypedef');
		self.load();
	});
	this.initUI();
}

WuIDE.prototype.initUI = function() {
	var root = $('#client');
	$('#menu').tabs();
}


WuIDE.prototype.load = function() {
	var self = this;
	var cont = $('#types');
	cont.empty();
	cont.append('<table id=type_list></table>');
	$.each(this.types, function(i,val) {
		$('#type_list').append('<tr id=type'+i+'><td>'+$(val).attr('name')+'</td><td><button id=type'+i+'_edit>Edit</button></td></tr>'); 
	});
	
	cont = $('#classes');
	cont.empty();
	cont.append('<table id=class_list></table>');
	$.each(this.classes, function(i,val) {
		$('#class_list').append('<tr id=type'+i+'><td>'+$(val).attr('name')+'</td><td><button id=type'+i+'_edit>Edit</button></td></tr>'); 
		self.installTypeEditor(i);
	});
}

WuIDE.prototype.installTypeEditor=function(i) {
	var self = this;
	$('#type'+i+'_edit').click(function() {
		self.editType(i);
	});
}

WuIDE.prototype.showEnumNameEditor = function(item,i) {
	$('body').append('<div id=enumname></div>');
	var dialog = $('#enumname');
	var enums = $(item).find('enum');
	dialog.append('<H2>Name</H2>');
	dialog.append('<input type=text id=enumname_text></input>');
	$('#enumname_text').val($(enums[i]).attr('value'));
	dialog.dialog({
		title: 'ENUM editor',
		autoOpen: true,
		buttons: {
			'OK': function () {
				// Update the XML here
				$('#type'+i+'_name').text($('#enumname_text').val());
				dialog.dialog('close');
				dialog.remove();
			},
			'Cancel': function() {
				dialog.dialog('close');
				dialog.remove();
			}
		}
	});
}

WuIDE.prototype.installEnumEditor = function(item,i) {
	var self = this;
	$('#type'+i+'_del').click(function() {
		$('#type'+i).remove();
		// remove item from XML
	});
	$('#type'+i+'_edit').click(function() {
		self.showEnumNameEditor(item,i);
	});
}

WuIDE.prototype.editType = function(i) {
	var self = this;
	$('#types').empty();
	var item = this.types[i];
	var enums = $(item).find('enum');
	
	$('#types').append('<button id=editdone>Done</button>');
	$('#types').append('<button id=addenum>Add</button>');
	$('#editdone').click(function() {
		// Update the XML here
		self.load();
	});
	$('#addenum').click(function() {
		var i = enums.length+1;
		$('#type_editor').append('<tr id=type'+i+'><td>Dummy</td><td><button id=type'+i+'_edit>Edit</button></td><td><button id=type'+i+'_del>Delete</button></td></tr>'); 
		$(item).append('<enum value=dummy></enum>');
		self.installEnumEditor(item,i);
	});
	$('#types').append('<table id=type_editor></table>');
	$.each(enums, function(i,val) {
		$('#type_editor').append('<tr id=type'+i+'><td id=type'+i+'_name>'+$(val).attr('value')+'</td><td><button id=type'+i+'_edit>Edit</button></td><td><button id=type'+i+'_del>Delete</button></td></tr>'); 
		self.installEnumEditor(item,i);
	});
}
