// vim: ts=4 sw=4
$(document).ready(function() {
	appid = getURLParameter('appid')
	try {
		ide = new WuIDE(g_user);
	} catch(e) {
		ide = new WuIDE(false);
	}
	$.fn.selectRange = function(start, end) {
		if(!end) end = start; 
		return this.each(function() {
			if (this.setSelectionRange) {
				this.focus();
				this.setSelectionRange(start, end);
			} else if (this.createTextRange) {
				var range = this.createTextRange();
				range.collapse(true);
				range.moveEnd('character', end);
				range.moveStart('character', start);
				range.select();
			}
		});
	};
});

function getURLParameter(name) 
{
    return decodeURIComponent((new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.search)||[,""])[1].replace(/\+/g, '%20'))||null;
}

function WuIDE(user)
{
    var self = this;
	this.is_user = user;
	this.updateTimeout = 5;
	if (user) {
		$.get('/componentxmluser?appid='+appid,function(r) {
			self.xml = $.parseXML(r);
			self.parseXML();
			self.load();
		});
	} else {
		$.get('/componentxml',function(r) {
			self.xml = $.parseXML(r);
			self.parseXML();
			self.parseEnableXML();
		});
	}
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
					self.classes[i].appCanCreateInstances=$(val).attr('appCanCreateInstances');
					self.classes[i].createInstancesAtStartup=$(val).attr('createInstancesAtStartup');
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
		var privateCData = $(val).attr("privateCData");
		var prop = [];
		$.each(properties, function(j, val) {
			var pname = $(val).attr('name');
			var datatype=$(val).attr('datatype');
			var access = $(val).attr('access');
			var def = $(val).attr('default');
			prop.push({name:pname, datatype:datatype, access:access, default:def});
		});
		self.classes.push({name:name, id:id, virtual:virtual,type:type,properties:prop,enabled:false,privateCData:privateCData});
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
		xml = xml + '    <WuClass name="'+self.classes[i].name+'" id="'+self.classes[i].id+'" virtual="'+self.classes[i].virtual+'" type="'+self.classes[i].type+'"';
		if (self.classes[i].privateCData)
		    xml = xml +' privateCData="'+self.classes[i].privateCData+'">\n';
		else
		    xml = xml + '>\n';
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
	return xml;
}

WuIDE.prototype.initUI = function() {
	var root = $('#client');
	$('#menu').tabs();
	$('#serialport_sel').click(function() {
		$.get('/serialport',function(r) {
			$('#serialport').empty();
			var list = $.parseJSON(r);
			$.each(list,function(i,val) {
				$('#serialport').append('<option val=port'+i+'>'+val+'</option>');
			});
		});
	});

	var self = this;
	$('#build').click(function() {
		$('#log').html("building...");
		$.get('/build', {cmd:'start'}, function(data) {
			$('#log').val(data).animate({scrollTop:$("#log")[0].scrollHeight - $("#log").height()});
			if (self.timer)
				clearTimeout(self.timer);
			self.bulidDone = false;
			self.refreshBuild();
		})
	});

	$('#upload').click(function() {
		var port = $('#serialport :selected').text();
		if(port == "") {
			$('#log').html("Please select the port");
			return;
		}
		$('#log').html("uploading...");
		$.get('/upload', {port: port,cmd:'start'}, function(data) {
			$('#log').val(data).animate({scrollTop:$("#log")[0].scrollHeight - $("#log").height()});
			if (self.timer)
				clearTimeout(self.timer)
			self.uploadDone = false;
			self.refreshUpload();
		})
	});	
	$('#upload_arduino').click(function() {
		$('#log').html("uploading to arduino...");
		$.get('/upload', {cmd:'start',target:'arduino'}, function(data) {
			$('#log').val(data).animate({scrollTop:$("#log")[0].scrollHeight - $("#log").height()});
			if (self.timer)
				clearTimeout(self.timer)
			self.uploadDone = false;
			self.refreshUpload();
		})
	});	
}



WuIDE.prototype.refreshBuild = function() {
	var self=this;
	
	if(self.buildDone) {
		return;
	}

	self.timer = setTimeout(function () {
		self.refreshBuild();
	},1000);
	
	$.get('/build', {cmd:'poll'}, function(data) {
		if (data != '') {
			if (self.data != data) {
				self.updateTimeout=5;
				$('#log').val(data).animate({scrollTop:$("#log")[0].scrollHeight - $("#log").height()});
			} else {
				if (self.updateTimeout <= 0)
					self.buildDone = true;
				self.updateTimeout--;
			}
			self.data = data;
		}
	});
}

WuIDE.prototype.refreshUpload = function() {
	var self=this;
	if(self.uploadDone) {
		return;
	}

	self.timer = setTimeout(function () {
		self.refreshUpload();
	},1000);

	$.get('/upload', {cmd:'poll'}, function(data) {
		if (data != '') {
			if (self.data != data) {
				self.updateTimeout=5;
				$('#log').val(data).animate({scrollTop:$("#log")[0].scrollHeight - $("#log").height()});
			} else {
				if (self.updateTimeout <= 0)
					self.uploadDone = true;
				self.updateTimeout--;
			}
			self.data = data;
	    }
	});
}


WuIDE.prototype.generateNewID = function() {
	var max = 0;

	$.each(this.classes, function(i,v) {
		var id = parseInt(v.id);
		if (id > max)
			max = id;
	});
	return max+1;
}
WuIDE.prototype.isIDUsed = function(id) {
	var i;

	for(i=0;i<this.classes.length;i++) {
		if (this.classes[i].id == id)
			return true;
	}
	return false;
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
	$('#addtype').unbind().click(function() {
		self.types.push({type:'enum',name:'New_Type',enums:[]});
		self.load();
	});
	$('#saveall').unbind().click(function() {
		$('#class_editor_done').trigger('click');
		$('#typeeditdone').trigger('click');
		setTimeout(function() {
			var xml = self.toXML();
			data = {xml:xml};
			if (ide.is_user) {
				$.post('/componentxmluser?appid='+appid, data);
			} else {
				$.post('/componentxml', data);
				var xml = '<WuKong>\n';
				$.each(self.classes,function(i,val) {
					if (val.enabled) {
						xml = xml + '    <WuClass name="'+val.name+'"';
						if (val.appCanCreateInstances)
							xml = xml + ' appCanCreateInstances="'+val.appCanCreateInstances+'"';
						if (val.createInstancesAtStartup)
							xml = xml + ' createInstancesAtStartup="'+val.createInstancesAtStartup+'"';
						xml = xml +' />\n';
					}
				});
				xml = xml + '</WuKong>';
				$.post('/enablexml', {xml:xml});
			}
		},500);
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
					},
					'button._del@id': function (arg) {
						return 'delclass'+self.classes[arg.pos].id;
					},
					'._enable@id': function(arg) {
						return 'class_check'+self.classes[arg.pos].id;
					},
				}
			}
		});
	}
	$('#classes').render(data,this.classListTemplate);
	$('#addclass').unbind().click(function() {
		self.classes.push({id:self.generateNewID(),enabled:false, name:'New_Class', type:'soft', virtual:true, properties:[]});
		self.load();
		$('#classes').show();
	});
	$.each(self.classes,function(i,val) {
		self.installClassEditor(i,val);
	});
	$('#class_list').show();
	$('#class_editor').hide();
}

WuIDE.prototype.installClassEditor=function(i,val) {
	var self = this;
	$('#class'+val.id).click(function() {
		var cls = new WuClass(val);
		cls.render('#class_editor');
		$('#class_list').hide();
		$('#class_editor').show();
	});
	$('#class_check'+val.id).change(function() {
		if ($('#class_check'+val.id).attr('checked'))
			val.enabled = true;
		else
			val.enabled = false;
	});
	$('#delclass'+val.id).click(function() {
		var classes=[];
		$.each(self.classes, function(i,v) {
			if (v.id != val.id) {
				classes.push(v);
			}
		});
		self.classes = classes;
		self.load();
		$('#classes').show();
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
	$('#typeeditcancel').unbind().click(function() {
		// Update the XML here
		$('#type_editor').hide();
		$('#type_list').show();
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
WuClass.prototype.installPropertyFunction=function(i,val) {
	var self = this;
	$('#propdel'+i).click(function() {
		properties=[];
		$.each(self.val.properties, function(i,v) {
			if (v.name != val.name)
				properties.push(v);
		});
		self.val.properties = properties;
		self.render('#class_editor');
		$('#class_editor').show();
	});
}

WuClass.prototype.updateClass=function() {
	var self=this;
	self.val.name = $('#class_editor_name').val();
	self.val.id = $('#class_editor_id').val();
	if ($('#class_editor_virtual').val() == 'y') {
		self.val.virtual = true;
	} else {
		self.val.virtual = false;
	}
	if ($('#class_editor_type').val() == 'h') {
		self.val.type = 'hard';
	} else {
		self.val.type = 'soft';
	}
	if ($('#class_editor_create').val() == 'y') {
		self.val.appCanCreateInstances = 'true';
	} else {
		self.val.appCanCreateInstances = null;
	}
	if ($('#class_editor_instance').val() == "")
		self.val.createInstancesAtStartup = null;
	else
		self.val.createInstancesAtStartup = $('#class_editor_instance').val();
	self.val.privateCData = $('#class_editor_private').val();
	if (self.val.privateCData == 'n')
		self.val.privateCData = null;
	else if (self.val.privateCData == 's')
		self.val.privateCData = 'short';
	else if (self.val.privateCData == 'b')
		self.val.privateCData = 'bool';

	$.each(self.val.properties,function(i,v) {
		v.name = $('#property'+i+' ._name').val();
		v.datatype = $('#property'+i+' ._datatype').val();
		if (v.datatype == 'Short')
			v.datatype = 'short';
		else if (v.datatype == 'Boolean')
			v.datatype = 'boolean';
		else if (v.datatype == 'refresh_rate')
			v.datatype = 'refresh_rate'

		v.access = $('#property'+i+' ._access').val();
		if (v.access == 'Write Only')
			v.access = 'writeonly';
		else if (v.access == 'Read/Write')
			v.access = 'readwrite';
		else if (v.access == 'Read Only')
			v.access = 'readonly';

		v.default = $('#property'+i+' ._default').val();
	});
}
WuClass.prototype.render=function(id) {
	var self = this;
	var datatype='<option val=b>Boolean</option>';
	datatype = datatype + '<option val=s>Short</option>';
	datatype = datatype + '<option val=r>refresh_rate</option>';


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
					},
					'._del@id': function(arg) {
						return 'propdel'+arg.pos;
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
		else if (val.datatype == 'refresh_rate')
			$('#property'+i+' ._datatype').val('refresh_rate');

		if (val.access == 'writeonly') 
			$('#property'+i+' ._access').val('Write Only');
		else if (val.access == 'readwrite')
			$('#property'+i+' ._access').val('Read/Write');
		else if (val.access == 'readonly')
			$('#property'+i+' ._access').val('Read Only');
		self.installPropertyFunction(i,val);
	});
	$('#class_editor_name').val(this.val.name);
	$('#class_editor_id').val(this.val.id);
	$('#class_editor_id').focusout(function() {
		var newid = $('#class_editor_id').val();
		if (self.val.id == newid) return
		if (ide.isIDUsed(newid)) {
			alert("This ID has been used");
			$('#class_editor_id').val(self.val.id);
		}
	});
	if (this.val.virtual == true)
		$('#class_editor_virtual').val('y');
	else 
		$('#class_editor_virtual').val('n');
	if (this.val.type == 'hard')
		$('#class_editor_type').val('h');
	else 
		$('#class_editor_type').val('s');
	if (this.val.appCanCreateInstances == 'true')
		$('#class_editor_create').val('y');
	else 
		$('#class_editor_create').val('n');
	
	$('#class_editor_instance').val(this.val.createInstancesAtStartup);
	if (this.val.privateCData == 'short')
		$('#class_editor_private').val('s');
	else if (this.val.privateCData == 'bool')
		$('#class_editor_private').val('b');
	else
		$('#class_editor_private').val('n');


	$('#class_editor_done').unbind().click(function() {
		$('#class_editor').hide();
		$('#class_list').show();
		self.updateClass();
		ide.load();
		$('#classes').show();
	});

	$('#class_editor_cancel').unbind().click(function() {
		$('#class_editor').hide();
		$('#class_list').show();
		ide.load();
		$('#classes').show();
	});

	$('#class_editor_edit').click(function() {
		var name = $('#class_editor_name').val();
        if (ide.is_user) {
            $.get('/wuclasssource?appid='+appid+'&src='+name+'&type='+$('#class_editor_lang').val(),function(r) {
                var code = {lang:$('#class_editor_lang').val(), code:r, name:name};
                TextEditor.load(code);
            });
        } else {
            $.get('/wuclasssource?src='+name+'&type='+$('#class_editor_lang').val(),function(r) {
                var code = {lang:$('#class_editor_lang').val(), code:r, name:name};
                TextEditor.load(code);
            });
        }
	});
	$('#addprop').unbind().click(function() {
		self.val.properties.push({id:self.val.properties.length,name:'myname',access:'readwrite',datatype:'boolean',default:''});
		self.updateClass();
		self.render('#class_editor');
		$('#class_editor').show();

	});
}


var TextEditor = new Object();

TextEditor.load=function(self) {
	$('#texteditor').show();
	$('#class_editor').hide();
	$('#texteditor_done').unbind().click(function() {
		$('#texteditor').hide();
		$('#editor').remove();
		$('#class_editor').show();
		if (ide.is_user)
			var arg={name:TextEditor.source.name, appid:appid,type:TextEditor.source.lang,content: TextEditor.editor.getValue()};
		else
			var arg={name:TextEditor.source.name, type:TextEditor.source.lang,content: TextEditor.editor.getValue()};
		$.post('/wuclasssource', arg,function () {
		});
	});
	$('#texteditor_cancel').unbind().click(function() {
		$('#texteditor').hide();
		$('#editor').remove();
		$('#class_editor').show();
	});
	$('#editor').remove();
	$('#texteditor').append('<div id=editor style="width:800px;height:800px"></div>');
	$('#editor').text(self.code);
	var ed = ace.edit('editor');
	ed.setTheme("ace/theme/twilight");
	if (self.lang == 'C') {
		ed.getSession().setMode('ace/mode/c_cpp');
	} else if (self.lang == 'Java') {
		ed.getSession().setMode('ace/mode/java');
	}
	TextEditor.editor = ed;
	TextEditor.source = self;
}
