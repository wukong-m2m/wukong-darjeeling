// vim: ts=4 sw=4
var FBP_canvas;
var FBP_CANVAS_TOP=0;
var FBP_CANVAS_LEFT=0;
var FBP_BLOCK_SIGNALSTOP = 26 + 22;
var FBP_BLOCK_SIGNALITEM_HEIGHT = 23;

var FBP_linkIsActive= false;
var g_lines=[];
var g_nodes=[];
var g_pages={};
var g_filename;
g_current_page = null;
var id=window.location.href;
var f=id.split("/");
id = f[4];
var currentApplication;
$(document).ready(function() {
    $.post("/applications/"+id, function(r) {
        g_filename = r.app.id;
        currentApplication = new WuKongApplication(r.app)
        currentApplication.renderForSidebar(sidebar.querySelector('.sidebar-tabcontent #applicationEditForm'))
        fbp2Init()
    });
})
function fbp2Init(){
    var $tabcontent = $('#sidebar .sidebar-tabcontent[tab="t2"]')
    var components = $tabcontent.find('#wuclasses');

/*  HY: comments out
    toolbar.append('<button id=toolbar_addBlock style="position: relative; left: 0px; top: -155px;">Add</button>');
    $('#toolbar_addBlock').click(function() {
        FBP_addBlock();
    });
    var toolbar = $('#toolbar')
    toolbar.append('<table><tr>');
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
*/

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

    //var viewport = updateCanvasHeight();
    //$('#canvastop').height(viewport.height).width(viewport.width);
    //$('#canvas').height(viewport.height).width(viewport.width);

    FBP_canvastop=$('#canvastop');
    FBP_canvas=$('#canvas');

    FBP_canvastop.hide();
    $('#connection').dialog({autoOpen: false});
    var mousedown = function(e) {
        var ratio = zoomRatio / 100
        var position = getViewportOffset()
        var px = e.pageX / ratio - position.left
        var py = e.pageY / ratio - position.top
        var line = Line_search(g_lines,px,py);
        if (line) {
            e.stopImmediatePropagation()
            $('#msg').html(line.toString());
            $('#msg').css('left',e.pageX).css('top',e.pageY+10).show();
            setTimeout(function() {
                $('#msg').hide();
            },2000);
            if (line === g_selected_line ) return;
        } else {
            $('#msg').hide();
        }
        last_selected_line = g_selected_line;
        g_selected_line = line;
        //FBP_link_cleanup()
        FBP_block_cleanup()
        if (last_selected_line){
            var erase = true
            last_selected_line.draw(FBP_canvas,{color:'red',width:4,erase:true})
            last_selected_line.draw(FBP_canvas)
            if (!g_selected_line) Line.clearPropertyEditForm()
        }
        if (g_selected_line){
            g_selected_line.draw(FBP_canvas,{color:'red',width:3});
            //render properties in sidebar
            g_selected_line.renderPropertyEditForm()
        }
    }
    FBP_canvas.mousedown(mousedown);
    $('#fileloader').dialog({autoOpen:false});
    $('#fileloader_file').val('fbp.sce');
    FBP_loadFromServer(id);
    window.progress = $('#progress');
    $('#progress').dialog({autoOpen:false, modal:true, width:'50%', height:'300'});
};

function FBP_deletePage()
{
    var yes = confirm('Are you sure ?')
    if (!yes) return;
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

        if (i==2) document.getElementById('deletePageBtn').setAttribute('disabled',1)
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
function FBP_fillBlockTypeInPage(pageTitle){
    var nodes = g_pages[pageTitle].nodes;
    nodes = nodes.slice()
    nodes.sort(function(a,b){
        try{
           var ta = Block.classes[a.type]
           var tb = Block.classes[b.type]
           return (ta > tb) ? 1 : (ta < tb ? -1 : 0)
        }
        catch(e){
           return -1
        }
    })
    var tags = [];
    for(i=0;i<nodes.length;i++) {
       var block = Block.classes[nodes[i].type]
       var iconClass = block.typename
       var icon = '<img src="'+block.icon44.src+'" class="wuClassIcon '+iconClass+'"/>'
       var location = nodes[i].location || 'No Location'
       var desc = '<span class="'+(nodes[i].location ? '' : 'warning-text')+'">'+location+'</span>'
       tags.push('<li><input type="radio" name="pagenode" value="'+(pageTitle+'\t'+nodes[i].id)+'">'+icon+'<label>'+nodes[i].type+'</label>'+desc+'</li>');
    }
    $('#toolbar_type_count').text(nodes.length);
    $div = $('#toolbar_type');
    $div.html(tags.join(''))
    $div.find('li').on('click',function(evt){
        var li = evt.currentTarget
        li.querySelector('input').checked = true;
    })
    $('#addBlockBtn')[0].onclick = function(evt){
        FBP_importBlock()
    }
}
function FBP_fillBlockTypeCategory($div,subcategory)
{
    if (typeof(subcategory)=='undefined'){
        subcategory = null;
        var tags = ['<option value="__all__">All</option>']
        for (var i in WuClassCategory.rootCategory.children){
            var category = WuClassCategory.rootCategory.children[i]
            tags.push('<option value="'+i+'">'+category.title+'</option>')
        }
        tags.push('<option value="__import__">In Other Pages</option>')
        $div.html(tags.join(''))
        // binding
        $div.on('change',function(evt){
            var el = evt.currentTarget
            var i = el.options[el.selectedIndex].value
            var subcategory;
            if (i=='__all__'){
                //pass
            }
            else if (i == '__import__'){
                var tags = []
                var initPageTitle;
                $.each(g_pages,function(title,val) {
                    if (title == g_current_page) return;
                    tags.push('<option val="'+title+'">'+title+'</option>');
                    if (!initPageTitle) initPageTitle = title
                });
                if (tags.length){
                    $('#toolbar_type_category1').html(tags.join(''))
                    FBP_fillBlockTypeInPage(initPageTitle)
                }
                else{
                    $('#toolbar_type_category1').html('<option>No other pages</option>')
                    FBP_fillBlockType($('#toolbar_type'),[])
                }
                return;
            }
            else subcategory = WuClassCategory.rootCategory.children[parseInt(i)]
            FBP_fillBlockTypeCategory($('#toolbar_type_category1'),subcategory)
        })
        // render next level
        FBP_fillBlockTypeCategory($('#toolbar_type_category1'),subcategory)
    }
    else{
        if (subcategory===null){
            // render all
            $div[0].innerHTML = '<option>All</option>'
            $div[0].setAttribute('disabled',1)
            var blocks = Block.getBlockTypes('__all__');
            FBP_fillBlockType($('#toolbar_type'),blocks)
        }
        else{
            var tags = ['<option value="__all__">All</option>']
            for (var i in subcategory.children){
                var category1 = subcategory.children[i]
                tags.push('<option value="'+i+'">'+category1.title+'</option>')
            }
            $div[0].innerHTML = tags.join('')
            $div[0].removeAttribute('disabled')
            var blocks = Block.getBlockTypes(subcategory);
            FBP_fillBlockType($('#toolbar_type'),blocks)
            $div.on('change',function(evt){
                var el = evt.currentTarget
                var i = el.options[el.selectedIndex].value
                var blocks;
                if (i=='__all__'){
                    blocks = Block.getBlockTypes(subcategory);
                }
                else{
                    blocks =  Block.getBlockTypes(null,subcategory.children[parseInt(i)]);
                }
                FBP_fillBlockType($('#toolbar_type'),blocks)
            })
        }
    }
}
function FBP_fillBlockType($div,blocks)
{
    $('#toolbar_type_count').text(blocks.length);
    if (blocks.length==0){
        $('#addBlockBtn')[0].setAttribute('disabled',1)
        $div.html('')
        return;
    }
    var i;
    var tags = []
    for(i=0;i<blocks.length;i++) {
//       div.append('<option val='+blocks[i]+'>'+blocks[i]+'</option>');
       //div.append('<option val='+blocks[i]+' ondblclick=FBP_addBlock()>'+blocks[i]+'</option>');
       var block = blocks[i]
       var iconClass = block.typename
       var icon = '<img src="'+(block.icon44 ? block.icon44.src : genericIcon.src)+'" class="wuClassIcon '+iconClass+'"/>'
       var desc = '<span>This is brief description of '+block.typename+'</span>'
       tags.push('<li><input type="radio" name="blocktype" value="'+block.typename+'">'+icon+'<label>'+block.typename+'</label>'+desc+'</li>');
    }
    $div.html(tags.join(''))
    $div.find('li').on('click',function(evt){
        var li = evt.currentTarget
        li.querySelector('input').checked = true;
        //show desc
        var typename = li.querySelector('input').value
        var fulldesc = 'this is full description of '+typename
        fulldesc += fulldesc
        fulldesc += fulldesc
        $('#wuclasses .fulldesc')[0].innerHTML = fulldesc.length+';'+fulldesc;
    })
    $('#addBlockBtn')[0].onclick = function(evt){
        FBP_addBlock()
    }
    $('#addBlockBtn')[0].removeAttribute('disabled')
}
/* HY: this is the original FBP_addBlock
function FBP_addBlock()
{
    var type = $('#toolbar_type').val();
    var block = Block.factory(type);
    // This should be replaced with the node type system latter
    block.attach($('#content'));
    g_nodes.push(block);
}
*/
function FBP_addBlock()
{
    var selected = document.querySelector('#toolbar_type li input[type="radio"]:checked')
    if (!selected) return;
    var type = selected.value
    var block = Block.factory(type);
    // This should be replaced with the node type system latter
    block.attach($('#content'));
    g_nodes.push(block);
    top.notifyApplicationContentTainted(true)
    var size = getViewportSize()
    var offset = getViewportOffset()
    block.setPosition(size.width/4,size.height/4)
    if (block.type=='Annotation') Block.setCurrent(block)
}

function FBP_deleteBlock()
{
    var yes = confirm('Are you sure?')
    if (!yes) return;
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
    if (!Block.current) return;
    for(i=0;i<g_nodes.length;i++) {
        if (g_nodes[i].id == Block.current.id) {
            g_nodes.splice(i,1);
            break;
        }
    }
    var new_lines = [];
    for(i=0;i<g_lines.length;i++) {
        if ((g_lines[i].source.id != Block.current.id)&&
            (g_lines[i].dest.id != Block.current.id)) {
            new_lines.push(g_lines[i]);
        }
    }
    g_lines = new_lines;
    FBP_refreshLines();
    Block.current.div.remove();
    Block.current = null;
    top.notifyApplicationContentTainted(true)
}
function FBP_deleteLink(){
    if (!g_selected_line) return;
    var yes = confirm('Are you sure?')
    if (!yes) return;
    var new_lines = [];
    for(i=0;i<g_lines.length;i++) {
        if (g_lines[i] !== g_selected_line) new_lines.push(g_lines[i]);
        /*
        if ((g_lines[i].source.id != Block.current.id)&&
            (g_lines[i].dest.id != Block.current.id)) {
            new_lines.push(g_lines[i]);
        }
        */
    }
    g_selected_line = null;
    g_lines = new_lines;
    FBP_refreshLines();
    document.getElementById('blockPropEditForm').innerHTML = ''
    sidebarActiveTab('t1')
    top.notifyApplicationContentTainted(true)
}
function FBP_refreshLines()
{
    FBP_canvas.clearCanvas();
    /*
    var ctx = FBP_canvas[0].getContext('2d');
    ctx.save();
ctx.setTransform(1,0,0,1,0,0);
// Will always clear the right space
ctx.clearRect(0,0,ctx.canvas.width,ctx.canvas.height);
ctx.restore();
*/
    for(i=0;i<g_lines.length;i++) {
        g_lines[i].draw(FBP_canvas);
    }
}
function FBP_buildConnection(source,sIndex, obj,aIndex)
{
    var i;
    $('#connection_src').empty();
    var slots = source.slots;
    var c = 0;
    for(i=0;i<slots.length;i++) {
        var sig = slots[i];
        if (!sig._readable) continue
        var selected = i==sIndex ? ' selected' : ''
        $('#connection_src').append('<option'+selected+' value='+c+'>'+sig.name+'</option>');
        c += 1;
    }
    slots = obj.slots;
    $('#connection_act').empty();
    c = 0
    for(i=0;i<slots.length;i++) {
        var act = slots[i];
        if (!act._writable) continue
        var selected = i==aIndex ? ' selected' : ''
        $('#connection_act').append('<option'+selected+' value='+c+'>'+act.name+'</option>');
        c += 1
    }
    $('#connection').dialog({
        buttons: {
            'Connect': function() {
                var sig = $('#connection_src').val();
                var act = $('#connection_act').val();
                /*
                console.log('SIN:'+sig)
                console.log(source.signals)
                console.log('ACT:'+act)
                console.log(obj.actions)
                */
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
function FBP_block_cleanup(){
    Block.setCurrent(null)
}
function FBP_link_cleanup ($target,_lastHit){
    FBP_linkIsActive = false;
    FBP_canvastop.clearCanvas()
    FBP_canvastop.unbind();
    FBP_canvastop.hide();
    FBP_source = null;
    if ($target) $target.removeClass('link-source')
    if (_lastHit) {
        _lastHit.div.removeClass('link-dest')
        _lastHit.div.removeClass('shadow')
    }
    FBP_canvastop[0].parentNode.onmousemove = null;
    FBP_canvastop[0].parentNode.onmousedown = null;
}

// when linking, don't pan or zoom
function FBP_link(evt)
{
    if (!Block.current) {
        alert("select a source first");
        return;
    }
    var $target = $(evt.currentTarget)
    var signalId = evt.currentTarget.getAttribute('id')
    var signalIndex = signalId.split('_')[2]
    var source = Block.current.slots[parseInt(signalIndex)]

    // the source item should be readable
    if (!source._readable) return;

    var _lastHit;
    if (FBP_linkIsActive) {
        FBP_link_cleanup($target,_lastHit)
        return;
    }
    $target.addClass('link-source')
    //let the red line starts from the clicked item
    FBP_linkIsActive = true;
    FBP_source = Block.current;
    var x1,y1,x2,y2;

    var pos = FBP_source.getAbsPosition();

    var size = FBP_source.getSize();

    x1 = pos[0]+size[0]/2;
    y1 = pos[1]+size[1]/2+signalIndex* FBP_BLOCK_SIGNALITEM_HEIGHT
    FBP_canvastop.show();

    var offset = getViewportOffset()
    var canvasandclient = $('#canvasandclient')
    var position = {left:parseFloat(canvasandclient.css('left')),top:parseFloat(canvasandclient.css('top'))}
    var leftOffset = offset.left;
    var topOffset =  offset.top;
    var ratio = zoomRatio /100
    var look4Link = function(e) {
        e.stopImmediatePropagation()
        x2 = e.pageX/ratio - offset.left
        y2 = e.pageY/ratio - offset.top
        //x2 = e.pageX;
        //y2 = e.pageY-FBP_CANVAS_TOP;
        FBP_canvastop.clearCanvas().drawLine({
            strokeStyle: '#3399ff',
            strokeWidth: 3,
            x1: x1, y1:y1,
            x2: x2, y2:y2
        });
        ///console.log([[e.pageX,e.pageY],[e.clientX,e.clientY],[x2,y2]])
        x2 += position.left-panOffset.left
        y2 += position.top-panOffset.top

        var obj = Block.hitTest(x2,y2)
        if (obj === FBP_source) return;
        if (obj && (_lastHit !== obj)) {
            obj.div.addClass('link-dest')
            obj.div.addClass('shadow')
            _lastHit = obj
        }
        else if ((!obj) && _lastHit){
            _lastHit.div.removeClass('link-dest')
            _lastHit.div.removeClass('shadow')
            _lastHit = null
        }
    };
    FBP_canvastop[0].parentNode.onmousemove = look4Link;
    FBP_canvastop[0].parentNode.onmousedown = function(e) {
        var mx = e.pageX/ratio - offset.left
        var my = e.pageY/ratio - offset.top
        mx += position.left-panOffset.left
        my += position.top-panOffset.top
        var obj = Block.hitTest(mx,my);
        if (obj && obj !== FBP_source) {
            //FBP_canvastop.unbind();
            //FBP_canvastop.hide();
            e.stopImmediatePropagation()
            var slot = obj.div[0].querySelector('.slot:hover')
            var actionIndex = slot.getAttribute('id').split('_')[2];
            FBP_buildConnection(FBP_source,signalIndex,obj,actionIndex);
            top.notifyApplicationContentTainted(true)
        }
        else if (!obj){
            // pan the workarea (canvas and client)
        }
        else{
            e.stopImmediatePropagation()
        }
        FBP_link_cleanup($target,_lastHit)
        //FBP_source = null;
    };

    $(document).on('EscKeyUp',function(){
        FBP_link_cleanup($target,_lastHit)
    })
}
function FBP_save_meta_or_content(){
    if (!(top._AppContentTainted_ || top._AppMetaTainted_)){
        return;
    }
    if (top._AppContentTainted_) FBP_save(function(success){
        if (success && !top._AppMetaTainted_){
            alert('Saving Succeed')
        }
    })
    if (top._AppMetaTainted_){
        //save name
        $.ajax({
            type:'PUT',
            dataType:'json',
            url:'/applications/'+currentApplication.id+'/rename',
            data:{value:currentApplication.app_name},
            success:function(data){
                //console.log(data)
                if (data.status==1){
                    alert('Failure:'+data.mesg)
                }
                else{
                    top.notifyApplicationMetaTainted(false)
                    alert('Saving Succeed')
                }
            }
        })
    }
}
function FBP_save(callback)
{
    var i;
    var data;
    loading(true)
    FBP_updatePage();

    data='<application name="'+g_filename+'">\n';

    //HY:save application.desc
    if (currentApplication.desc != '') data += '<desc>'+encodeURIComponent(currentApplication.desc)+'</desc>'

    //HY:save canvas pan
    if (panOffset.left || panOffset.top){

        data += '<pan>'+panOffset.left+','+panOffset.top+'</pan>'
        //console.log('save panoffset to '+panOffset.left+','+panOffset.top)

    }
    if (zoomRatio != 100){
        data += '<zoom>'+zoomRatio+'</zoom>'
    }

    $.each(g_pages, function(title,obj) {
        data += '<page title="'+title+'">\n';
        data += FBP_toXML(g_pages[title].nodes, g_pages[title].lines);
        data += '</page>\n';
    });
    data = data + "</application>\n";
    //console.log('----save----')
    //console.log(data)

    $.ajax({
        url:'/applications/'+id+'/fbp/save',
        data: {xml:data},
        type:'POST',
        success: function(data) {
            //window.progress.dialog({buttons:{}});
            //window.progress.dialog('open');
            //FBP_waitSaveDone(id, data.version);
            loading(false)
            top.notifyApplicationContentTainted(false)
            if (callback) callback(true)
        }
    });
}

function FBP_download(callback){
    //test:
    //http://192.168.56.101:5000/applications/f45b0e6aec165544faccaf2cad820542/fbp/download
    var url = '/applications/'+id+'/fbp/download'
    open(url,'application+'+id)
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

    //HY:restore desc
    var $desc = app.find('desc')
    if ($desc.length){
        currentApplication.desc = decodeURIComponent($desc.text())
        document.querySelector('.application-property textarea[name="desc"]').value = currentApplication.desc
    }
    //HY:restore zoom
    var $zoom = app.find('zoom')
    if ($zoom.length){
        setTimeout(function(){
            zoomRatio = parseInt($zoom.text())
            var style = document.querySelector('#canvasandclient').style
            style.zoom = zoomRatio+'%'
            $('.zoomCtrl span').text(zoomRatio+'%')
        },100)
    }
    //HY:restore pan
    var $pan = app.find('pan')
    if ($pan.length){
        setTimeout(function(){
            //move the pan after all lines were drawed
            var pan = $pan.text().split(',')
            panOffset = {left:parseFloat(pan[0]),top:parseFloat(pan[1])}
            var vs = getViewportSize()
            var style = document.querySelector('#canvasandclient').style
            var canvasboxstyle = document.querySelector('#canvasbox').style
            var left = -vs.width + panOffset.left
            var top = -vs.height + panOffset.top
            //var left = (-vs.width + panOffset.left-parseInt(canvasboxstyle.left)*(zoomRatio/100))
            //var top =   (-vs.height + panOffset.top-parseInt(canvasboxstyle.top)*(zoomRatio/100))
           style.left = left + 'px'
           style.top = top+'px'
            console.log('set panoffset to '+panOffset.left+','+panOffset.top+' by '+left+','+top)
        },100)

    }
    var pages = app.find("page");
    var comps;
    var i,len,j;
    if (pages.length<2) document.getElementById('deletePageBtn').setAttribute('disabled',1)
    if (pages.length == 0) {
        g_pages={};
        comps = $(xml).find("component");
        g_pages['_first_'] = FBP_parseXMLPage(comps);
        if (g_current_page == null){
            g_current_page = '_first_'
        }
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
    //reset Category of WuClasses (Components)
    FBP_fillBlockTypeCategory($('#toolbar_type_category0'));

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
        meta.name = c.attr('name') || c.attr('type')


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
                    meta.monitorProper[attr.value] = attr.name;
                }
            }
        }

        loc = c.find("location");
        if (loc.length > 0) {
            meta.location = loc.attr("requirement").replace('&amp;','&').replace('&tilde;','~');
        }
        //console.log(c.find("group_size").attr("requirement"));
        group_size = c.find("group_size");
        if (group_size.length > 0) {
            meta.group_size = group_size.attr("requirement");
        } else {
            meta.group_size = 1;
        }
        reaction_time = c.find("reaction_time")
        if (reaction_time.length > 0) {
            meta.reaction_time = reaction_time.attr("requirement");
        } else {
            meta.reaction_time = 1;
        }
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
    //$('#pagelist').append('<option value="newpage">New Page</option>');
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
            FBP_addPage()
        } else {

            FBP_updatePage();
            g_current_page = sel;
            FBP_renderPage(g_pages[g_current_page]);
        }
    });

}
function FBP_addPage(){
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
                document.getElementById('deletePageBtn').removeAttribute('disabled')
            },
            'Cancel': function() {
                $('#dianewpage').dialog("close");
                $('#dianewpage').remove();
            }
        }
    }).dialog("open");

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
                    console.log(meta.nodes[i])
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
        var name = (source.name && source.name != source.type) ? ' name="'+source.name+'"' : '';
        xml = xml + '    <component type="'+source.type+'" instanceId="'+source.id+'" x="'+source.x+'" y="'+source.y+'" w="'+source.w+'" h="'+source.h+'"'+name+'>\n';
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
            var l;  var sig;
            $.each(source.signals, function(name,val) {
                xml = xml + name + '="'+val+'" ';
            });
            xml = xml + ' />\n';
        }
//sato add end
        if(source.monitorProper) {
            xml = xml +'        <monitorProperty ';
            for(var i in source.monitorProper) {
                xml = xml + source.monitorProper[i] + '="'+i+'" ';
            }
            xml = xml +'/>\n';
        }
        xml = xml + '    </component>\n';
    }
    for(var k in gnodes) {
        var source =gnodes[k];
        if (linehash[source.id] == undefined) {
            var name = (source.name && source.name != source.type) ? ' name="'+source.name+'"' : '';
            xml = xml + '    <component type="'+source.type+'" instanceId="'+source.id+'" x="'+source.x+'" y="'+source.y+'" w="'+source.w+'" h="'+source.h+'"'+name+'>\n';
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
                var l;  var sig;
                $.each(gnodes[k].signals, function(name,val) {
                    xml = xml + name + '="'+val+'" ';
                });
                xml = xml + ' />\n';
            }
//sato add end
            if(source.monitorProper) {
                xml = xml +'        <monitorProperty ';
                for(var i in source.monitorProper) {
                    xml = xml + source.monitorProper[i] + '="'+i+'" ';
                }
                xml = xml +'/>\n';
            }
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




/* HY: this is original FBP_importBlock()
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
        if (sel == undefined) {
            return;
        }
        var nodes = g_pages[sel].nodes;
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
*/
function FBP_importBlock(){
    var selected = document.querySelector('#toolbar_type li input[type="radio"]:checked')
    if (!selected) return;
    var page_nid = selected.value.split('\t')
    var pageTitle = page_nid[0]
    var nid = parseInt(page_nid[1])
    var nodes = g_pages[pageTitle].nodes
    var node;
    for (var i in nodes){
        node = nodes[i]
        if (node.id == nid) break
    }
    var block = Block.restore(node);
    block.attach($('#content'));
    g_nodes.push(block);
}

/*deployment*/
var _DepolymentDialogCloseHandler;
function FBP_openDepolymentDialog(callback){
    var size = getViewportSize()
    //position to center
    $('#deployment').css({
        'display':'block',
        'left':size.width*0.035,
        'top':size.height *0.035
    })
    _DepolymentDialogCloseHandler = callback
}
function FBP_closeModalDialog(){
    $('.modal-dialog').css({
        'display':'none'
    })
    if (_DepolymentDialogCloseHandler) _DepolymentDialogCloseHandler()
}
function FBP_currentNode(){
    FBP_openDepolymentDialog()
    var task = function(){
        $.post('/nodes/refresh/0', function(data) {
            $('#deployment .modal-dialog-content').html(data.nodes);
            $('#deployment .modal-dialog-content #refresh').on('click',function(){
               FBP_closeDepolymentDialog()
               setTimeout(function(){FBP_currentNode()},50)
            })
        });
    }
    task()
}
function FBP_deploy(){
    loading(true)
    $('#deployment .modal-dialog-content').html('Deploy Starts...')
    $.post('/applications/' + currentApplication.id + '/deploy', function(data) {
        // Already an object
        loading(false)
        FBP_openDepolymentDialog(function(){
            top.stop_polling()
        })
        if (data.status == 1) {
            alert(data.mesg);
        } else {
            // Print deploy status to #deploy-progress
            // Starts a new polling to deploy-progress
            var property = 'deploy_status'
            var count = 0
            var zfill3 = function(n){
                if (n>99) return n
                else if (n>9) return '0'+n
                return '00'+n
            }
            setTimeout(function() {
                top.application_polling(currentApplication.id, '#deploy-progress', property,function(data){
                    var tags = []
                    for (var i=0; i<data[property].length; i++) {
                        count += 1
                        tags.push("<pre>"+zfill3(count)+":[" + data[property][i].level + "] " + data[property][i].msg + "</pre>");
                    }
                    tags.reverse()
                    $('#deployment .modal-dialog-content').prepend(tags.join(''))
                });
            },3000);
        }
    });
}
function FBP_map(){
    loading(true)
    $.post('/applications/' + currentApplication.id + '/deploy/map', function(data) {
        loading(false)
        FBP_openDepolymentDialog()
        // Already an object
        if (data.status == 1) {
            alert(data.mesg);
        } else {
            $('#deployment .modal-dialog-content').html('Mapping Progress:<div id="mapping-progress"></div>Mapping Results:<div id="mapping_results"><table style="width:100%"></table></div>')
            var $table = $('#mapping_results table');
            $table.empty();
            var results = data.mapping_results
            for (var i=0,l=results.length;i<l;i++){
                var result = results[i]
                var instances = result.instances
                if (instances.length==0){
                    $table.append('<tr class=error><td>'+result.instanceId+'</td><td>'+result.name+'</td><td>'+result.msg+'</td><td></td></tr>');
                }
                else{
                    for (var j=0,k=instances.length;j<k;j++){
                        var instance = instances[j]
                        $table.append('<tr class=info><td>'+instance.instanceId+'</td><td>(Virtual) '+instance.name+'</td><td>'+instance.nodeId+'</td><td>'+instance.portNumber+'</td></tr>');
                    }
                }
            }

            // print mapping status to #mapping-progress
            $('#mapping-progress').empty();
            for (var i=0; i<data.mapping_status.length; i++) {
              $('#mapping-progress').append("<pre>[" + data.mapping_status[i].level + "]" + data.mapping_status[i].msg + "</pre>");
            }

            // disable deploy button if mapping is not successful
            if (!data.mapping_result) {
              $('button#deployBtn')[0].setAttribute('disabled',1)
            } else {
              $('button#deployBtn')[0].removeAttribute('disabled')
            }
        }
    });
}

