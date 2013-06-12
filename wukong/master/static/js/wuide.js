// vim: ts=4 sw=4
$(document).ready(function() {
	ide = new WuIDE();
});



function WuIDE()
{
    var self = this;
	$.get('/componentxml',function(r) {
		self.xml = $.parseXML(r);
		self.parseXML();
		self.parseEnableXML();
	});
	this.initUI();
}

WuIDE.prototype.parseEnableXML = function() {
	var self = this;
	$.get('/enablexml',function(r) {
		var xml = $.parseXML(r);
		var classes = $(xml).find('WuClass');
		$.each(classes, function(i,val) {
			for(i=0;i<self.classes.length;i++) {
				if (self.classes[i].name == $(val).attr('name')) {
					self.classes[i].enabled = true;
					return;
				}
			}
		});
		self.load();
	});
}

WuIDE.prototype.parseXML = function() {
	var classes =$(this.xml).find('WuClass');
	var types =$(this.xml).find('WuTypedef');
	var self = this;

	self.classes = [];
	$.each(classes, function(i,val) {
		var name = $(val).attr('name');
		var id = $(val).attr('id');
		var virtual = $(val).attr('virtual');
		var type = $(val).attr('type');
		var properties = $(val).find('property');
		var prop = [];
		$.each(properties, function(j, val) {
			var pname = $(val).attr('name');
			var datatype=$(val).attr('datatype');
			var access = $(val).attr('access');
			var def = $(val).attr('default');
			prop.push({name:pname, datatype:datatype, access:access, default:def});
		});
		self.classes.push({name:name, id:id, virtual:virtual,type:type,properties:prop,enabled:false});
	});
	self.types=[];
	$.each(types,function(i,v) {
		var name=$(v).attr('name');
		var type=$(v).attr('type');
		var enumlist=[];
		if (type == 'enum') {
			var enums = $(v).find('enum');
			$.each(enums,function(j,v) {
				enumlist.push({name:$(v).attr('value')});
			});
			self.types.push({name:name,type:type,enums:enumlist});
		}
	});
}

WuIDE.prototype.toXML = function() {
	var xml = '<WuKong>\n';
	var i;
	var self=this;

	for(i=0;i<self.types.length;i++) {
		xml = xml + '    <WuTypedef name="'+self.types[i].name+'" type="enum">\n';
		for(j=0;j<this.types[i].enums.length;j++) {
			xml = xml + '        <enum value="'+self.types[i].enums[j].name+'"/>\n';
		}
		xml = xml + '    </WuTypedef>\n';
	}
	for(i=0;i<self.classes.length;i++) {
		xml = xml + '    <WuClass name="'+self.classes[i].name+'" id="'+self.classes[i].id+'" virtual="'+self.classes[i].virtual+'" type="'+self.classes[i].type+'">\n';
		for(j=0;j<this.classes[i].properties.length;j++) {
			
			xml = xml + '        <property name="'+self.classes[i].properties[j].name+'" ';
			xml = xml + 'access="'+self.classes[i].properties[j].access+'" ';
			xml = xml + 'datatype="'+self.classes[i].properties[j].datatype+'" ';
			if (self.classes[i].properties[j].default)
				xml = xml + 'default="'+self.classes[i].properties[j].default+'" ';
			xml = xml + ' />\n';
		}
		xml = xml + '    </WuClass>\n';
	}
	xml = xml + '</WuKong>\n';
	alert(xml);
}

WuIDE.prototype.initUI = function() {
	var root = $('#client');
	$('#menu').tabs();
}


WuIDE.prototype.load = function() {
	var self = this;
	var cont = $('#types');
	var i;

	var data = {types:self.types,classes:self.classes};
	if (self.typelistTemplate == null) {
		this.typelistTemplate=$('#type_list').compile({
			'tr._type': {
				'type <- types': {
					'td._name':'type.name',
					'button._edit@id': function(arg) {
						return 'type'+arg.pos+'_edit';
					},
					'button._del@id':function(arg) {
						return 'type'+arg.pos+'_del';
					}
				}
			}
		});
	}
	$('#type_list').empty();
	$('#type_list').render(data,this.typelistTemplate);
	$.each(self.types,function(i,val) {
		self.installTypeEditor(i);
	});
	$('#addtype').click(function() {
		self.types.push({type:'enum',name:'New Type',enums:[]});
		self.load();
	});
	$('#saveall').click(function() {
		self.toXML();
	});
		
	
	if (self.classListTemplate == null) {
		this.classListTemplate=$('#classes').compile({
			'tr._class': {
				'class <- classes': {
					'td._name':'class.name',
					'._enable@checked': function(arg) {
						return self.classes[arg.pos].enabled;
					},
					'button._edit@id': function (arg) {
						return 'class'+self.classes[arg.pos].id;
					}
				}
			}
		});
	}
	$('#classes').render(data,this.classListTemplate);
	$.each(self.classes,function(i,val) {
		self.installClassEditor(val);
	});
	$('#class_list').show();
	$('#class_editor').hide();
}

WuIDE.prototype.installClassEditor=function(val) {
	var self = this;
	$('#class'+val.id).click(function() {
		var cls = new WuClass(val);
		cls.render('#class_editor');
		$('#class_list').hide();
		$('#class_editor').show();
	});
}


WuIDE.prototype.installTypeEditor=function(i) {
	var self = this;
	$('#type'+i+'_edit').click(function() {
		self.editType(i);
	});
	$('#type'+i+'_del').click(function() {
		self.types.splice(i,1);
		self.load();
	});
}

WuIDE.prototype.showEnumNameEditor = function(item,i) {
	$('body').append('<div id=enumname></div>');
	var dialog = $('#enumname');
	dialog.append('<H2>Name</H2>');
	dialog.append('<input type=text id=enumname_text></input>');
	$('#enumname_text').val(item.enums[i].name);
	dialog.dialog({
		title: 'ENUM editor',
		autoOpen: true,
		buttons: {
			'OK': function () {
				// Update the XML here
				item.enums[i].name = $('#enumname_text').val();
				$('#enum'+i+' td._name').text($('#enumname_text').val());
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
	$('#delenum'+i).click(function() {
		item.enums.splice(i,1);
		$('#type_editor').empty();
		$('#type_editor').render(item, self.editTypeTemplate);
		$.each(item.enums, function(i,val) {
			self.installEnumEditor(item,i);
		});
		$('#type_editor').show();
	});
	$('#editenum'+i).click(function() {
		self.showEnumNameEditor(item,i);
	});
}

WuIDE.prototype.refreshEnumList = function(item,i) {
	var self = this;
	if (this.editTypeTemplate == null) {
		this.editTypeTemplate = $('#type_editor').compile({
			'tr._class': {
				'e <- enums': {
					'td._name':'e.name',
					'button._edit@id': function(arg) {
						return 'editenum'+arg.pos;
					},
					'button._del@id': function(arg) {
						return 'delenum'+arg.pos;
					},
					'.@id': function(arg) {
						return 'enum'+arg.pos;
					}
				},
			},
		});
	}
	$('#type_editor').empty();
	$('#type_editor').render(item, this.editTypeTemplate);
	$.each(item.enums, function(i,val) {
		self.installEnumEditor(item,i);
	});
	$('#typeeditdone').unbind().click(function() {
		// Update the XML here
		$('#type_editor').hide();
		$('#type_list').show();
		item.name = $('#type_editor_name').val();
		self.load();
	});
	$('#addenum').unbind().click(function() {
		item.enums.push({name:'Dummy'});
		self.refreshEnumList(item,i);
	});
	$('#type_editor').show();
}
WuIDE.prototype.editType = function(i) {
	var self = this;
	var item = this.types[i];
	var enums = item.enums;
	$('#type_list').hide();
	this.refreshEnumList(item,i);
	$('#type_editor_name').val(this.types[i].name);
}



function WuClass(val)
{
	this.val = val;
}

WuClass.prototype.render=function(id) {
	var self = this;
	var datatype='<option val=b>Boolean</option>';
	datatype = datatype + '<option val=s>Short</option>';


	if (WuClass.propertyTemplate == null) {
		WuClass.propertyTemplate = $(id).compile({
			'tr._property': {
				'p <- properties': {
					'.@id': function(arg) {
						return 'property'+arg.pos;
					},
					'._name@value': 'p.name',
					'._datatype': function(arg) {
						return datatype;
					},
					'._default': function(arg) {
						if (self.val[arg.pos].default)
							return self.val[arg.pos].default;
						else
							return '';
					}
				}
			}
		});
	}
	$(id).empty();
	$(id).render(self.val, WuClass.propertyTemplate);
	$.each(self.val.properties,function(i,val) {
		if (val.datatype == 'boolean')
			$('#property'+i+' ._datatype').val('Boolean');
		else if (val.datatype == 'short')
			$('#property'+i+' ._datatype').val('Short');
		if (val.access == 'writeonly') 
			$('#property'+i+' ._access').val('Write Only');
		else if (val.access == 'readwrite')
			$('#property'+i+' ._access').val('Read/Write');
		else if (val.access == 'readonly')
			$('#property'+i+' ._access').val('Read Only');
	});
	$('#class_editor_name').val(this.val.name);
	$('#class_editor_id').val(this.val.id);
	if (this.val.virtual == 'true')
		$('#class_editor_virtual').val('y');
	else 
		$('#class_editor_virtual').val('n');
	if (this.val.type == 'hard')
		$('#class_editor_type').val('h');
	else 
		$('#class_editor_type').val('s');
	$('#class_editor_done').unbind().click(function() {
		$('#class_editor').hide();
		$('#class_list').show();
	});
	$('#class_editor_edit').click(function() {
		var code = {lang:$('#class_editor_lang').val(), code:'xxxx'};
		TextEditor.load(code);
	});
}


var TextEditor = new Object();

TextEditor.load=function(self) {
	$('#texteditor').show();
	$('#class_editor').hide();
	$('#texteditor_done').unbind().click(function() {
		$('#texteditor').hide();
		$('#class_editor').show();
		self.code = _cp.getCode();
	});
	editor.edit(self.code,self.lang);
}
