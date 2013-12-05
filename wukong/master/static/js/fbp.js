// vim: ts=4 sw=4
var FBP_canvas;
var FBP_CANVAS_TOP=50;
var FBP_linkIsActive= false;
var g_lines=[];
var g_nodes=[];
var g_pages={};
var g_filename;
g_current_page = null;
var id=window.location.href;
var f=id.split("/");
id = f[4];
$.post("/applications/"+id, function(r) {
    g_filename = r.app.id;
});
$(document).ready(function() {
    $('#client').append('<div id=toolbar></div>');
    $('#client').append('<div id=content></div>');
    var toolbar = $('#toolbar');
	toolbar.append('<table><tr>');
    toolbar.append('<td><select id=toolbar_type size=10 style="width:200px;"></select></td>');
    FBP_fillBlockType($('#toolbar_type'));

/*    toolbar.append('<button id=toolbar_addBlock style="position: relative; left: 0px; top: -155px;">Add</button>');
    $('#toolbar_addBlock').click(function() {
        FBP_addBlock();
    });
*/
    toolbar.append('<td valign="top"><button id=toolbar_editComponent>Edit Component</button></td>');
    $('#toolbar_editComponent').click(function() {
        FBP_editComponent();
    });
    toolbar.append('<td valign="top"><button id=toolbar_importBlock>Import</button></td>');
    $('#toolbar_importBlock').click(function() {
        FBP_importBlock();
    });
    toolbar.append('<td valign="top"><button id=toolbar_delBlock>Del</button></td>');
    $('#toolbar_delBlock').click(function() {
        FBP_delBlock();
    });
    toolbar.append('<td valign="top"><button id=toolbar_link>Link</button></td>');
    $('#toolbar_link').click(function() {
        FBP_link();
    });
    toolbar.append('<td valign="top"><button id=toolbar_save>Save</button></td>');
    $('#toolbar_save').click(function() {
        FBP_save();
    });
    toolbar.append('<td valign="top"><button id=toolbar_deletepage>Delete Page</button></td>');
    $('#toolbar_deletepage').click(function() {
        FBP_deletePage();
    });
	toolbar.append('<td valign=top><select id=pagelist></select></td>');
    toolbar.append('</tr></table>');

    /*
    toolbar.append('<button id=toolbar_load>Load</button>');
    $('#toolbar_load').click(function() {
    FBP_load();
    });
    toolbar.append('<button id=toolbar_activate>Activate</button>');
    $('#toolbar_activate').click(function() {
    FBP_activate(g_nodes,g_lines);
    });
    */
    FBP_canvastop=$('#canvastop');
    FBP_canvas=$('#canvas');
    $('#canvastop').hide();
    $('#connection').dialog({autoOpen: false});
    $('#canvas').mousedown(function(e) {
        var px = e.pageX;
        var py = e.pageY;
        var l = Line_search(g_lines,px,py);
        if (l != null) {
            $('#msg').html(l.toString());
        } else {
            $('#msg').html('');
        }
        $('#msg').css('left',px).css('top',py).show();
        setTimeout(function() {
            $('#msg').hide();
        },2000);
		tmp = g_selected_line;
		g_selected_line = l;
		if (tmp)
			tmp.draw(FBP_canvas);
		if (g_selected_line)
			g_selected_line.draw(FBP_canvas);
    });
    $('#fileloader').dialog({autoOpen:false});
    $('#fileloader_file').val('fbp.sce');
    FBP_loadFromServer(id);
    window.progress = $('#progress');
    $('#progress').dialog({autoOpen:false, modal:true, width:'50%', height:'300'});
});

function FBP_deletePage()
{
	var name='';
	var i=0;

	$.each(g_pages,function(k,v) {
		i = i + 1;
		if (name == '' && k != g_current_page)
			name = k;
	});
	if (i > 1) {
		g_disable_page_update = true;
		delete g_pages[g_current_page];
		g_current_page = name;
		FBP_initPage();
		FBP_renderPage(g_pages[g_current_page]);
		g_disable_page_update = false;
	}
}

function FBP_editComponent()
{
	$('body').append('<div id=edit_component></div>');
	$('#edit_component').append('<iframe width=100% height=90% src=/static/editcomponent.html?appid='+id+'></iframe>');
	$('#edit_component').dialog({
		width:'80%',
		height:800,
		buttons:{
			'OK': function() {
				$('#edit_component').dialog('close');
				$('#edit_component').remove();
			}
		}
	});
}

function FBP_fillBlockType(div)
{
    var blocks = Block.getBlockTypes();
    var i;

    for(i=0;i<blocks.length;i++) {
//       div.append('<option val='+blocks[i]+'>'+blocks[i]+'</option>');
       div.append('<option val='+blocks[i]+' ondblclick=FBP_addBlock()>'+blocks[i]+'</option>');
    }
}

function FBP_addBlock()
{
    var type = $('#toolbar_type').val();
    var block = Block.factory(type);
    // This should be replaced with the node type system latter
    block.attach($('#content'));
    g_nodes.push(block);
}

function FBP_delBlock()
{
	if (Block.current == null) return;
	if (g_selected_line) {
		var lines=[];
		for(i=0;i<g_lines.length;i++) {
			if (g_lines[i] != g_selected_line) {
				lines.push(g_lines[i]);
			}
		}
		g_lines = lines;
		FBP_refreshLines();
		g_selected_line = null;
		return;
	}

	var new_lines = [];
    for(i=0;i<g_lines.length;i++) {
		if ((g_lines[i].source.id != Block.current.id)&&
		    (g_lines[i].dest.id != Block.current.id)) {
			new_lines.push(g_lines[i]);
		}
	}
    for(i=0;i<Block.widgets.length;i++) {
        if (Block.widgets[i].id == Block.current.id) {
            // FIXME: need to update widgets as well, but this should not be a separate list, there should be one list to manage all the blocks
            Block.widgets.splice(i,1)
            break;
        }
    }
    // Should've delete links before blocks, it seems like that's what happening
    // otherwise the links will be updated fully
    for(i=0;i<g_nodes.length;i++) {
        if (g_nodes[i].id == Block.current.id) {
            g_nodes.splice(i,1);
            break;
        }
    }
	g_lines = new_lines;
	FBP_refreshLines();
    Block.current.div.remove();
    Block.current = null;
    
}

function FBP_refreshLines()
{
    FBP_canvas.clearCanvas();
    for(i=0;i<g_lines.length;i++) {
        g_lines[i].draw(FBP_canvas);
    }
}
function FBP_buildConnection(source, obj)
{
    var i;
    var list = source.getSignals();

    $('#connection_src').empty();
    for(i=0;i<list.length;i++) {
        var sig = list[i];
        $('#connection_src').append('<option value='+i+'>'+sig.name+'</option>');
    }
    list = obj.getActions();
    $('#connection_act').empty();
    for(i=0;i<list.length;i++) {
        var act = list[i];
        $('#connection_act').append('<option value='+i+'>'+act.name+'</option>');
    }
    $('#connection').dialog({
        buttons: {
            'Connect': function() {
                var sig = $('#connection_src').val();
                var act = $('#connection_act').val();
                var l = new Line(source,source.signals[sig].name,obj,obj.actions[act].name);
                g_lines.push(l);
                FBP_refreshLines();
                $('#connection').dialog("close");
            },
            'Cancel': function() {
                $('#connection').dialog("close");
            }
        }
    }).dialog("open");

}

function FBP_link()
{
    if (Block.current == null) {
        alert("select a source first");
        return;
    }
	if (FBP_linkIsActive) {
		FBP_linkIsActive = false;
		$('#canvastop').unbind();
		$('#canvastop').hide();
		return;
	}
	FBP_linkIsActive = true;
    FBP_source = Block.current;
    var x1,y1,x2,y2;

    var pos = FBP_source.getPosition();
    var size = FBP_source.getSize();
    x1 = pos[0]+size[0]/2;
    y1 = pos[1]+size[1]/2-FBP_CANVAS_TOP;
    $('#canvastop').show();

    $('#canvastop').mousemove(function(e) {
        x2 = e.pageX;
        y2 = e.pageY-FBP_CANVAS_TOP;
        FBP_canvastop.clearCanvas().drawLine({
            strokeStyle: "#000",
            strokeWidth: 4,
            x1: x1, y1:y1,
            x2: x2, y2:y2
        });
    });
    $('#canvastop').mousedown(function(e) {
        var obj = Block.hitTest(e.pageX,e.pageY);
        if (obj) {
            $('#canvastop').unbind();
            $('#canvastop').hide();
            FBP_buildConnection(FBP_source, obj);
        }
        FBP_source = null;
    });
}
function FBP_save()
{
    var i;
	var data;

	FBP_updatePage();
	
    data='<application name="'+g_filename+'">\n';
	$.each(g_pages, function(title,obj) {
		data += '<page title="'+title+'">\n';
	    data += FBP_toXML(g_pages[title].nodes, g_pages[title].lines);
		data += '</page>\n';
	});
    data = data + "</application>\n";
    $.ajax({
        url:'/applications/'+id+'/fbp/save',
        data: {xml:data},
        type:'POST',
        success: function(data) {
            //window.progress.dialog({buttons:{}});
            //window.progress.dialog('open');
            //FBP_waitSaveDone(id, data.version);
        }
    });
}

function FBP_waitSaveDone(id, version)
{
    $.post('/applications/'+id+'/fbp/poll', {version: version}, function(data) {
        $('#progress #compile_status').html('<p>' + data.compile_status + '</p>');
        $('#progress #normal').html('<h2>NORMAL</h2><pre>' + data.normal.join('\n') + '</pre>').scrollTop(-1);
        $('#progress #urgent_error').html('<h2>URGENT</h2><pre>' + data.error.urgent.join('\n') + '</pre>');
        $('#progress #critical_error').html('<h2>CRITICAL</h2><pre>' + data.error.critical.join('\n') + '</pre>');

        if (data.compile_status < 0) {
            FBP_waitSaveDone(id, data.version);
        }
        else {
            $('#progress').dialog({buttons: {Ok: function() {
                $(this).dialog('close');
            }}})
        }
    });
}

function FBP_parseXML(r)
{
    var xml = $.parseXML(r);	
    var app = $(xml).find("application");
	var pages = app.find("page");
    var comps;
    var i,len,j;

	if (pages.length == 0) {
		g_pages={};
		comps = $(xml).find("component");
		g_pages['_first_'] = FBP_parseXMLPage(comps);
		if (g_current_page == null)
			g_current_page = '_first_'
	} else {
		g_pages={};
		for(i=0;i<pages.length;i++) {
			var page = $(pages[i]);
			comps = page.find("component");
			g_pages[page.attr('title')] = FBP_parseXMLPage(comps);
			if (g_current_page == null)
				g_current_page = page.attr('title');
		}
	}
}

function FBP_renderPage(page)
{
    $('#content').empty();
    g_nodes = [];
    g_lines = [];
    var hash={};
    var loc, group_size, reaction_time;

    for(i=0;i<page.nodes.length;i++) {
        n = Block.restore(page.nodes[i]);
        // This should be replaced with the node type system latter
        n.attach($('#content'));
        hash[n.id] = n;
        g_nodes.push(n);
    }
    for(i=0;i<page.lines.length;i++) {
	    var line = new Object();
		line.source = hash[page.lines[i].source];
		line.signal = page.lines[i].signal;
		line.dest = hash[page.lines[i].dest];
		line.action = page.lines[i].action;
        g_lines.push(Line.restore(line));
    }
    FBP_refreshLines();
}

function FBP_parseXMLPage(comps)
{
    nodes = [];
    lines = [];
    var hash={};
    var loc, group_size;

    for(i=0;i<comps.length;i++) {
        var c = $(comps[i]);
        var meta ={};
        meta.x = c.attr("x");
        meta.y = c.attr("y");
        meta.w = c.attr("w");
        meta.h = c.attr("h");
        meta.id = c.attr("instanceId");
        meta.type = c.attr("type");
        

        meta.sigProper = {};
        var properties = c.find("signalProperty");
        if(properties.length > 0) {
            var attrs = properties[0].attributes;
            if(attrs) {
                for(j=0;j<attrs.length;j++) {
                    var attr = attrs[j];
                    meta.sigProper[attr.name] = attr.value;
                }
            }
        }

        meta.monitorProper = {};
        var properties = c.find("monitorProperty");
        if(properties.length > 0) {
            var attrs = properties[0].attributes;
            if(attrs) {
                for(j=0;j<attrs.length;j++) {
                    var attr = attrs[j];
                    meta.monitorProper[attr.name] = attr.value;
                }
            }
        }

        loc = c.find("location");
        if (loc.length > 0) {
            meta.location = loc.attr("requirement").replace('&amp;','&').replace('&tilde;','~');
        }
        //console.log(c.find("group_size").attr("requirement"));
        console.log('location ' + meta.location);
        group_size = c.find("group_size");
        if (group_size.length > 0) {
            meta.group_size = group_size.attr("requirement");
        } else {
            meta.group_size = 1;
        }
        console.log('group size ' + meta.group_size);
        reaction_time = c.find("reaction_time")
        if (reaction_time.length > 0) {
            meta.reaction_time = reaction_time.attr("requirement");
        } else {
            meta.reaction_time = 1;
        }
        console.log('reaction time ' + meta.reaction_time);
		nodes.push(meta);
		hash[meta.id] = meta;
    }
    for(i=0;i<comps.length;i++) {
        var c = $(comps[i]);
        var links = c.find("link");
        for(j=0;j<links.length;j++) {
            var l = $(links[j]);
            var source = c.attr("instanceId");
            var signal = l.attr("fromProperty");
            var dest = l.attr("toInstanceId");
            var action = l.attr("toProperty");
            var a = {};
            a.source = hash[source].id;
            a.signal = signal;
            a.dest = hash[dest].id;
            a.action = action;
            lines.push(a);
        }
    }
	return {nodes:nodes,lines:lines};
}
var g_disable_page_update=false;
function FBP_initPage()
{
	$('#pagelist').empty();
	$('#pagelist').append('<option value="newpage">New Page...</option>');
	$.each(g_pages, function(title,page) {
		if (title == g_current_page) 
			$('#pagelist').append('<option selected value="'+title+'">'+title+'</option>');
		else
			$('#pagelist').append('<option value="'+title+'">'+title+'</option>');
	});
	$('#pagelist').change(function() {
		if (g_disable_page_update) {
			g_disable_page_update = false;
			return;
		}
		var sel = $('#pagelist option:selected').val();

		if (sel == 'newpage') {
			$('#content').append('<div id=dianewpage></div>');
			$('#dianewpage').append('<div>Please input the name of the new page</div><input type=text id=dianewpage_name></input>');

			$('#dianewpage').dialog({
				autoOpen:true, 
				modal:true, 
				width:400, height:200,
				buttons: {
					'Add': function() {
						var page = $('#dianewpage_name').val();
						if (g_pages.hasOwnProperty(page)) {
							alert('page title is duplicated');
							return;
						}
						FBP_updatePage();
						var option = new Option(page,page,true,true);
						$('#pagelist').append(option);
						$('#dianewpage').dialog("close");
						$('#dianewpage').remove();
						g_disable_page_update = true;
						g_current_page = page;
						g_pages[page] = {nodes:[], lines:[]};
						FBP_renderPage(g_pages[page]);
						g_disable_page_update = false;
					},
					'Cancel': function() {
						$('#dianewpage').dialog("close");
						$('#dianewpage').remove();
					}
				}
			}).dialog("open");
		} else {
			
			FBP_updatePage();
			g_current_page = sel;
			FBP_renderPage(g_pages[g_current_page]);
		}
	});

}

function FBP_updatePage()
{
	var nodes = [];
	var lines=[];
	var i,k;

	for(i=0;i<g_nodes.length;i++) {
		meta={};
		g_nodes[i].serialize(meta);
		// Update copied of the same component in all pages
		for(p in g_pages) {
			if (p == g_current_page) continue;
			var gnodes = g_pages[p].nodes;
			for(k=0;k < gnodes.length;k++) {
				if (gnodes[k].id == meta.id) {
					Block.copyData(gnodes[k], meta);
				}
			}
		}
		nodes.push(meta);
	}
	for(i=0;i<g_lines.length;i++) {
		meta={}
		line = g_lines[i].serialize();
		lines.push(line);
	}

	g_pages[g_current_page] = {nodes: nodes, lines: lines};
}

function FBP_loadFromServer(id)
{
    $.ajax({
        url:'/applications/'+id+'/fbp/load',
        type: 'POST',
        error: function(msg) {
            alert(msg)
        },
        success: function(r) {
            if (console) console.log(r);
			g_current_page = null;
            FBP_parseXML(r.xml);
			FBP_initPage();

			FBP_renderPage(g_pages[g_current_page]);
        }
    });
}

function FBP_load()
{
    $('#fileloader').dialog({buttons: {
        'OK': function() {
            g_filename = $('#fileloader_file').val();
            $.ajax({
                url:'/cgi-bin/file?load='+$('#fileloader_file').val(),
                success: function(r) {
                    meta = JSON.parse(r);
                    $('#content').empty();
                    g_nodes = [];
                    g_lines = [];
                    var hash={};
                    for(i=0;i<meta.nodes.length;i++) {
                        n = Block.restore(meta.nodes[i]);
                        // This should be replaced with the node type system latter
                        n.attach($('#content'));
                        hash[n.id] = n;
                        g_nodes.push(n);
                    }
                    for(i=0;i<meta.lines.length;i++) {
                        var a = meta.lines[i];
                        a.source = hash[a.source];
                        a.dest = hash[a.dest];
                        g_lines.push(Line.restore(a));
                    }
                    FBP_refreshLines();
                }
            });
            $('#fileloader').dialog("close");
        },
        'Cancel': function() {
            $('#fileloader').dialog("close");
        }
    }});
    $('#fileloader').dialog("open");
}


function FBP_serialize(gnodes,glines)
{
    var i;
    var len = gnodes.length;
    var meta={};
    var nodes = [];
    for(i=0;i<len;i++) {
        nodes.push(gnodes[i].serialize({}));
    }
    meta['nodes'] = nodes;
    len = glines.length;
    var lines=[];
    for(i=0;i<len;i++) {
        lines.push(glines[i].serialize());
    }
    meta['lines'] = lines;
    return meta;
}
function FBP_toXML(gnodes,glines)
{
    var linehash={};
	var hash={};
    var i;
    var len = glines.length;

    for(i=0;i<len;i++) {
        if (linehash[glines[i].source]) {
            linehash[glines[i].source].push(glines[i]);
        } else {
            linehash[glines[i].source] =[(glines[i])];
        }
    }
    for(var k in gnodes) {
		hash[gnodes[k].id] = gnodes[k];
	}
	var xml = '';
    for(var k in linehash) {
        var source =hash[linehash[k][0].source];
        xml = xml + '    <component type="'+source.type+'" instanceId="'+source.id+'" x="'+source.x+'" y="'+source.y+'" w="'+source.w+'" h="'+source.h+'">\n';
        len = linehash[k].length;
        for(i=0;i<len;i++) {
            var line = linehash[k][i];
            xml = xml + '        <link fromProperty="'+line.signal+'" toInstanceId="'+line.dest+'" toProperty="'+line.action+'"/>\n';
        }
        if (source.location && source.location != '') {
            xml = xml + '        <location requirement="'+source.location.replace('&','&amp;')+'" />\n';
        }
        if (source.group_size && source.group_size != '') {
            xml = xml + '        <group_size requirement="'+source.group_size+'" />\n';
        }
        if (source.reaction_time && source.reaction_time != '') {
            xml = xml + '        <reaction_time requirement="'+source.reaction_time+'" />\n';
        }
//sato add start            
		if(source.signals) {
			xml = xml + '        <signalProperty ';
			var l;	var sig;
			$.each(source.signals, function(name,val) {
				xml = xml + name + '="'+val+'" ';
			});
			xml = xml + ' />\n';
		}
//sato add end
        xml = xml + '    </component>\n';
    }
    for(var k in gnodes) {
        var source =gnodes[k];
        if (linehash[source.id] == undefined) {
            xml = xml + '    <component type="'+source.type+'" instanceId="'+source.id+'" x="'+source.x+'" y="'+source.y+'" w="'+source.w+'" h="'+source.h+'">\n';
            if (gnodes[k].location && gnodes[k].location != '') {
                xml = xml + '        <location requirement="'+source.location.replace('&','&amp;')+'" />\n';
            }
        	if (source.group_size && source.group_size != '') {
            	xml = xml + '        <group_size requirement="'+source.group_size+'" />\n';
	        }
        	if (source.reaction_time && source.reaction_time != '') {
            	xml = xml + '        <reaction_time requirement="'+source.reaction_time+'" />\n';
	        }
           
//sato add start            
			if(gnodes[k].signals) {
				xml = xml + '        <signalProperty ';
				var l;	var sig;
				$.each(gnodes[k].signals, function(name,val) {
					xml = xml + name + '="'+val+'" ';
				});
				xml = xml + ' />\n';
			}
//sato add end
            xml = xml + '    </component>\n';
        }
    }

    return xml;
}

function Signal(name)
{
    this.name = name;
	this.index = 0;
}

function Action(name)
{
    this.name = name;
	this.index = 0;
}




function FBP_importBlock()
{
	$('#diaimport').remove();
	$('#content').append('<div id=diaimport></div>');
	var pages = '<select id="diaimport_page">';
	var init=false;
	$.each(g_pages,function(title,val) {
		if (title == g_current_page) return;
		if (init == false) {
			pages = pages + '<option selected val="'+title+'">'+title+'</option>';
			init = true;
		} else {
			pages = pages + '<option val="'+title+'">'+title+'</option>';
		}
	});
	pages = pages + '</select>';
	$('#diaimport').append('<div>Import component from other page</div>'+pages+'<select id="diaimport_comp"></select>');
	$('#diaimport_page').change(function() {
		var sel = $('#diaimport_page option:selected').val();
		var list = $('#diaimport_comp');
		list.empty();
		var nodes = g_pages[sel].nodes;
		if (sel == undefined) {
			return;
		}
		for(i=0;i<nodes.length;i++) {
			list.append('<option val="'+nodes[i].id+'">'+nodes[i].id+','+nodes[i].type+','+nodes[i].location+'</option>');
		}
	});
	$('#diaimport_page').trigger("change");

	$('#diaimport').dialog({
		autoOpen:true, 
		modal:true, 
		width:600, height:400,
		buttons: {
			'Import': function() {
				var page = $('#diaimport_page option:selected').val();
				var comp = $('#diaimport_comp option:selected').val().split(',')[0];
				var nodes = g_pages[page].nodes;
				for(i=0;i<nodes.length;i++) {
					if (nodes[i].id == comp) break;
				}
				var meta = g_pages[page].nodes[i];
				if (meta == null) {
					alert('internal error');
					return;
				}
				var block = Block.restore(meta);
				block.attach($('#content'));
				g_nodes.push(block);
				$('#diaimport').dialog("close");
			},
			'Cancel': function () {
				$('#diaimport').dialog("close");
			}
		},
	}).dialog("open");
}
