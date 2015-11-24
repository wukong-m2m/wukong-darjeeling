// vim: ts=4 sw=4
var Block_count=0;
Block.linkStart = null;
Block.widgets=[];
Block.classes={};
function Block()
{
    this.init();
}
var genericIcon = new Image()
genericIcon.onload = function(){
    var icons = document.querySelectorAll('.wuClassIcon.GenericIcon, .wuClassIcon22.GenericIcon')
    for (var i in icons){
        icons[i].src = genericIcon.src;
    }
}
genericIcon.src =  '/static/images/wuclass/Generic.png';
function loadIcon(){
    var block = this.block
    var typename = this.typename
    var _icon44 = new Image()
    _icon44.onload = function(){
        block.icon44 = this;
        var icons = document.querySelectorAll('.wuClassIcon.wuClass'+typename+'Type, .wuClassIcon22.wuClass'+typename+'Type')
        for (var i in icons){
            icons[i].src = this.src;
        }
    }
    _icon44.onerror = function(){
        block.icon44 = false
    }
    _icon44.src = '/static/images/wuclass/'+typename+'.png'
}
Block.register = function (type, b) {
    Block.classes[type] = b;
    b.typename = type;
    //HY:temporary assign category attribute
    //   see datamodel.js for registerWuClassCategory()
    registerWuClassCategory(b)
    // Load Icons
    b.icon44 = genericIcon;
    loadIcon.apply({block:b,typename:type})
};
Block.prototype.icon44 = genericIcon;
Block.prototype.init=function() {
    this.id = Block_count;
    this.div = $('<div></div>');
    this.type='block';
    this.name = this.type;
    this.div.attr('id','obj'+this.id);
    this.div.css('position','absolute');
    //this.div.css('cursor','pointer');
    this.div.css('background-color','rgb(255,255,255,1)');
    //this.div.css('-webkit-box-shadow','2px 2px 2px #AAA');
    //this.div.css('-moz-box-shadow','2px 2px 2px #AAA');
    //this.div.css('box-shadow','2px 2px 2px #AAA');
    this.div.css('border:solid 1px #aaa');
    this.div.css('border-radius','2px');
    this.div.css('-webkit-border-radius','2px');
    this.div.css('-moz-border-radius','2px');
    this.div.attr('class','wublock');
    this.setPosition(0,0);
//  this.setPosition(Math.floor((Math.random()*100)),Math.floor((Math.random()*50)));
//  this.setSize(120,100);
    this.setLocation('')
    this.group_size = 1;
    this.reaction_time = 1;
    this.signals=[];
    this.actions=[];
    this.slots=[];
    this.sigProper=[];
    this.actProper=[];
    this.monitorProper={};
    this.numSlot = 0;
    Block_count++;
    Block.widgets.push(this);
}

Block.getBlockTypes=function(category0,category1) {
    var types=[];
    if (typeof(category0)=='undefined' || category0=='__all__'){
        for(var k in Block.classes) types.push(Block.classes[k]);
    }
    else if (typeof(category1)=='undefined' ||category1=='__all__'){
        for (var i in category0.children){
            var subcategory = category0.children[i]
            types = types.concat(subcategory.members)
        }
    }
    else{
        /*
        for(var k in Block.classes){
            if (Block.classes[k].category != category1) continue;
            types.push(Block.classes[k]);
        } */
        types = category1.members
    }
    types.sort(function(a,b){
        return a.typename > b.typename ? 1 : -1;
    })
    return types;
}

Block.factory=function(type) {
    var i;
    var cls = Block.classes[type];
    var block;
    if (cls){
        block = new cls();
        block.icon44 = cls.icon44
    }
    else block = new Block();
    return block;
}

Block.copyData=function(dest,src) {
    dest.location = src.location;
    dest.group_size = src.group_size;
    dest.reaction_time = src.reaction_time;
    dest.signals = src.signals;
    dest.monitor = src.monitor;
    dest.sigProper = src.signals;
    dest.monitorProper = src.monitorProper;
    // find a serial name
    var name = src.name


}

Block.prototype.setLocation = function(location){
    this.location = location
    if (this.location && this.location.length){
        var p = this.location.split('#');
        this.location_path = p[0]
        this.location_func = p.length > 1 ? p[1] : ''
    }
    else{
        this.location_path = ''
        this.location_func = ''
    }
}

Block.prototype.addMonitorProperty = function(index, property_name) {
    this.monitorProper[index] = property_name;
}

Block.prototype.removeMonitorProperty = function(index) {
    if(this.monitorProper[index]) {
        delete this.monitorProper[index];
    }
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
    obj.signals = this.sigProper;
    obj.monitorProper = this.monitorProper;
    obj.name = this.name
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
    n.setSize(a.w,a.h)
    n.type = a.type;
    //HY:assign name
    n.name = a.name || a.type
    n.setLocation(a.location);
    n.group_size = a.group_size;
    n.reaction_time = a.reaction_time;
    n.sigProper = a.sigProper;
    n.monitorProper = a.monitorProper;

    // Call the restore of the derived class in the future
    return n;
}
Block.prototype.setSlotRWProperty = function(){
    /* HY: add _readable, _writable property to this block*/

    /* check if we have done this already */
    if (typeof(this.slots[0]._readable) != 'undefined') return;

    var signals = this.getSignals()
    var actions = this.getActions()
    var dict = {}
    for (var i=0,l=signals.length;i<l;i++){
        dict[signals[i].name] = 1
    }
    for (var i=0,l=actions.length;i<l;i++){
        var v = dict[actions[i].name]
        dict[actions[i].name] = v ? 3 : 2;// 1,1 or 1,0
    }
    var slots = this.slots
    for(i=0;i<slots.length;i++) {
        var slot = slots[i]
        slot._readable = dict[slot.name] & 1
        slot._writable = dict[slot.name] & 2
    }
}
Block.prototype.draw=function() {
    // Annotation type is special wuclass
    if (this.type=='Annotation') return this.drawAnnotation()

    var i;
    var pos = this.getPosition();

    var size = this.getSize();
    this.div.empty();
    var tags = []
    var iconClass = this.type
    var type = this.type
    //var name = (this.name || type.replace('_',' '))
    var name = (this.name==this.type ? '&nbsp;' : this.name)
    var icon;
    if (this.icon44){
        icon = '<img src="'+this.icon44.src+'" class="wuClassIcon22 wuClass'+type+'Type"/>'
    }
    else{
        icon = '<block class="default-icon22">'+type.substr(0,1)+'</block>'
    }
    tags.push('<div class="block-type">'+icon + type + '</div>');
    if (type=='NodeREDAgent'){
        tags.push('<div class="block-name" blockid="'+this.id+'"><a class=" NodeREDAgent">Edit Node-RED Subgraph</a></div>')
    }
    else{
        tags.push('<div class="block-name" blockid="'+this.id+'">'+name+'</div>')
    }
    tags.push('<div class="block-signals">');

    this.setSlotRWProperty()

    var signalIds = []
    var slots = this.slots;
    var sidx = -1, aidx = -1;

    var readwritable = []
    var writeonly = []
    var readonly = []
    for(i=0;i<slots.length;i++) {
        var signalId = 'signal_'+this.id+'_'+i
        signalIds.push(signalId)
        var slot = slots[i]
        var divclass = ['slot'];
        if (slot._readable) divclass.push('readable')
        if (slot._writable) divclass.push('writable')
        var readableMark =  slot._readable ? '<span class="slottail">▶</span>' : ''
        var writableMark = slot._writable ? '◉' : ''
        tags.push('<div'+(slot._readable ? ' readable="1"' : '')+(slot._writable ? ' writable="1"' : '')+' class="'+divclass.join(' ')+'" id="'+signalId+'"><span class="slothead">'+writableMark+'</span><span class="slotbody"> '+slot.name.replace('_', ' ')+'</span>'+readableMark+'</div>');
    }
    //add to DOM
    this.div.append(tags.join('')+'</div>')

    //assign data and binding

    // when the block item was clicked
    var clickHandler = function(evt) {
        var $signal = $(evt.currentTarget)
        // ignore the click event when user clicks outside the
        // current focus block, only if this is a link destination
        if (!(Block.current && (Block.current.id == $signal.data('block_id')))) {
            return;
        }
        if (ShiftKeyDown){
            //user wanna monitor some property
            if(!$signal.hasClass("monitor_enable")) {
                $signal.addClass("monitor_enable");
                Block.current.addMonitorProperty($signal.data('index'), $signal.data('property_name'));
            } else {
                $signal.removeClass("monitor_enable");
                Block.current.removeMonitorProperty($signal.data('index'));
            }
            top.notifyApplicationContentTainted(true)
        }else{
            //user wanna make a link
            FBP_link(evt)
        }
    }
    for(i=0;i<slots.length;i++) {
        var $signal = $('#'+signalIds[i])
        $signal.data('property_name', this.slots[i].name);
        $signal.data('index', i);
        $signal.data('block_id', this.id);
        if(this.monitorProper[i] != undefined) {
            $signal.addClass("monitor_enable");
        }
        $signal.on('click', clickHandler);
    }
    // implement "Edit in Node-RED"
    if (type=='NodeREDAgent'){
        var a= document.querySelector('div.block-name[blockid="'+this.id+'"] a.NodeREDAgent');
        a.onclick = function(evt){
            evt.preventDefault;
            evt.stopPropagation();
            window.top.showNodeRedFrame()
        }
    }

}
Block.prototype.drawAnnotation = function(){
    this.div.html('<div class="slothead">&nbsp;</div><div class="content"></div>')
    var content = this.sigProper['content']
    this.div.find('div.content').text(content ? decodeURIComponent(content) : 'Text')
    this.div.addClass('annotation')
    this.div.css('background-color','')
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
//  this.signals[con]=type;
    return con;
}
Block.prototype.getSignals=function() {
    return this.signals;
}
Block.prototype.getActions=function() {
    return this.actions;
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
    return con;
//  this.actions[con]=type;
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
            return this.actions[i].index;
    }
    return -1;
}

Block.prototype.loadSourceCode=function(parent) {
    var obj = $('#propertyeditor_editor_area');
    var source = '';
    source = 'package javax.wukong.virtualwuclasses;\n';
    source = source + 'import javax.wukong.wkpf.WKPF;\n';
    source = source + 'import javax.wukong.wkpf.VirtualWuObject;\n';
    source = source + 'public class Virtual'+this.type+'WuObject extends GENERATEDVirtualThresholdWuObject {\n';
    source = source + '    public Virtual'+this.type+'WuObject() {\n';
    source = source + '        // Initialize the wuobject here\n';
    source = source + '    }\n';
    source = source + '    public void update() {\n';
    source = source + '        // CHeck the update of the properties here\n';
    source = source + '    }\n';
    source = source + '}\n';
    obj.text(source);
}
Block.setCurrent = function(block){
    if (block == Block.current) {
        if (block) sidebarActiveTab('t3')
        return;
    }
    if (Block.current){
        try {
            Block.current.div.resizable("destroy");
        } catch(e) {
        }
        Block.current.setFocus(false);
    }
    Block.current = block;
    if (!block) return;//cleanup the Block.current
    block.renderPropertyEditForm()
    block.div.resizable({
        stop: function( event, ui ) {
            top.notifyApplicationContentTainted(true)
        }
    });
    block.setFocus(true);
}
Block.prototype.attach=function(parent) {
    parent.append(this.div);
    this.div.draggable();
    var self = this;
    this.div.on('click',function(evt) {
        evt.stopImmediatePropagation()
        evt.preventDefault()
        Block.setCurrent(self)
    });
    this.div.bind("dragstop", function(event,ui) {
        var pos = self.getPosition()
        /*
        if (pos[0]<0) {
            self.setPosition(0,pos[1])
            pos[0] = 0
        }
        if (pos[1]<0){
            self.setPosition(pos[0],0)
            pos[1] = 0
        }
        */
        FBP_refreshLines();
        top.notifyApplicationContentTainted(true)
    });

    /*
    Disabled by HY (this.div.dblclick)
    Because the propertyEditor has been moved to sidebar

    this.div.dblclick(function() {
        var locationReqLst = undefined;
        if (self.location && self.location.length > 0) {
            locationReqLst = self.location.split('#');
        } else {
            locationReqLst = ['',''];
        }
        $('#propertyeditor').empty();
        $('#propertyeditor').append('<div id=propertyeditor_tab>');
        $('#propertyeditor_tab').append('<ul><li><a href=#propertyeditor_loc>Location Policy</a></li><li style="display:none"><a href=#propertyeditor_ft>Fault Tolerance</a></li><li><a href=#propertyeditor_default>Default Value</a></li><li id=context_tab><a href=#propertyeditor_context>Context</a></li><li style="display:none"><a href=#propertyeditor_monitor>Monitors</a></li></ul>');

        $('#propertyeditor_tab').append('<div id=propertyeditor_loc>Location: <input type=text id=propertyeditor_location_hierarchy style="width:300px"></input>'+
                                        '<button class="chooseLocNode" for="propertyeditor_location_hierarchy">Choose Tree Node</button><br>'+
                                        'Function: <input type=text id=propertyeditor_location_function style="width:300px"></input><br>'+
                                        'Functions Supported: use, range, farthest, closest, ~, |, &</div>');

        $('#propertyeditor_tab').append('<div id=propertyeditor_ft style="dislay:none"><label for="propertyeditor_groupsize">Group Size</label>');
        $('#propertyeditor_ft').append('<br><input id=propertyeditor_groupsize name=value></input>');
        $('#propertyeditor_ft').append('<br>');
        $('#propertyeditor_ft').append('<label for="propertyeditor_reactiontime">Reaction Time</label>');
        $('#propertyeditor_ft').append('<br><input id=propertyeditor_reactiontime name=value></input></div>');
        $('#propertyeditor_ft').append('');

        $('#propertyeditor_location_hierarchy').val(locationReqLst[0]);
        $('#propertyeditor_location_function').val(locationReqLst[1]);
        $('#propertyeditor_groupsize').spinner();
        $('#propertyeditor_groupsize').spinner("value",self.group_size);
        $('#propertyeditor_reactiontime').spinner();
        $('#propertyeditor_reactiontime').spinner("value",self.reaction_time);


        $("#propertyeditor_tab").append('<div id=propertyeditor_default></div>');
        $("#propertyeditor_tab").append('<div id=propertyeditor_context></div>');
        $("#propertyeditor_tab").append('<div id=propertyeditor_monitor></div></div>');
        $("#propertyeditor_default").empty();
        $("#propertyeditor_context").empty();
        $("#propertyeditor_monitor").empty();

        if (Block.current.type != "Plugin") {
            $("#context_tab").css("display", "none");
        } else {
            $("#propertyeditor_tab li").removeClass("ui-tabs-active");
            $("#propertyeditor_tab li").removeClass("ui-state-active");
            $("#context_tab").addClass("ui-tabs-active");
            $("#context_tab").addClass("ui-state-active");
            $.post('/contexts', function(data) {
                var contexts = jQuery.parseJSON(data);
                var selector = '<select id="context_select" multiple="multiple">';
                for(var key in contexts) {
                    selector += '<option value="' + key +'">' + contexts[key] + '</option>';
                }
                selector += '</select>';
                $("#propertyeditor_context").append(selector);
                $('#context_select').css({'width': 300});
                $('#context_select').multipleSelect();
                $("#context_select").multipleSelect("setSelects", Block.current.contexts);
            });
        }

        $("#propertyeditor_tab").tabs();

        var _siglist = Block.current.getSignals();
        for(i=0;i<_siglist.length;i++) {
            var sig = _siglist[i];
            $('#propertyeditor_default').append(sig.name + ":");
            $('#propertyeditor_default').append('<input type=text id=s'+sig.name+'></input><br>');
            try {
                $('#s'+sig.name).val(self.sigProper[sig.name]);
            } catch (e) {

            }
        }

        $('.chooseLocNode').click(function (){
            var inputId = $(this).attr('for');
            $.post('/loc_tree', function(data) {
                tree_html = top.generate_tree(data,"locTreeNodeInDialog");

                $('#treeInDialogDiv').empty();
                $('#treeInDialogDiv').html(tree_html);
                $('.locTreeNodeInDialog').click(function () {
                    var location_str='';
                    var clickedNodeId = parseInt($(this).attr("id"), 10);
                    while (clickedNodeId != 0) {
                        location_str = '/' + $("#"+clickedNodeId).text() + location_str;
                        clickedNodeId = Math.floor(clickedNodeId/100);
                    }
                    $('#selectedTreeNode').val(location_str);
                    $('#'+$('#tree_dialog').get(0).dataset.setFor).val(location_str);
                });
                $('#confirm_tree_dialog').click(function(){
                    $('#'+$('#tree_dialog').get(0).dataset.setFor).val($('#selectedTreeNode').val());
                });
                $('.dialogCloseButton').click(function() {
                    $('#tree_dialog').dialog("close");
                });
            });
            $('#display').treeview({
                collapsed: true,
                animated: "fast",
            });
            $('#tree_dialog').get(0).dataset.setFor = inputId;

            $('#tree_dialog').dialog();
            $('#tree_dialog').draggable();
            $('#tree_dialog').show();
        });

        $('#propertyeditor').dialog({
            buttons: {
                'OK': function () {
                    self.location = $('#propertyeditor_location_hierarchy').val()+'#'+$('#propertyeditor_location_function').val();
                    self.group_size = $('#propertyeditor_groupsize').spinner("value");
                    self.reaction_time = $('#propertyeditor_reactiontime').spinner("value");
                    self.contexts = $("#context_select").multipleSelect("getSelects");
                    for(i=0;i<_siglist.length;i++){
                        sig = _siglist[i];
                        self.sigProper[sig.name]=$('#s'+sig.name).val();
                    }
                    $('#propertyeditor').dialog("close");

                },
                'Cancel': function() {
                    $('#propertyeditor').dialog("close");
                }
            },
            width:'90%', height:400,
            title:"Property Editor"

        }).dialog("open");
    });
    */
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
    var size = getViewportSize()
    x = parseFloat(x) + size.width
    y = parseFloat(y) + size.height
    this.div.css('left',x).css('top',y);
}

Block.prototype.setSize=function(w,h) {
    this.div.css('width',w).css('height',h);
}
Block.prototype.getDIV=function() {
    return this.div;
}

Block.prototype.getPosition=function() {
    var x = Block.getpx(this.div.css('left'))
    var y = Block.getpx(this.div.css('top'))
    var size = getViewportSize()
    x = parseFloat(x) - size.width
    y = parseFloat(y) - size.height
    return [x,y];
}
Block.prototype.getAbsPosition=function() {
    var x = Block.getpx(this.div.css('left'))
    var y = Block.getpx(this.div.css('top'))
    return [x,y]
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
/* HY's version of PropertyEditor */
Block.prototype.renderPropertyEditForm = function(){
    if (this.type=='Annotation') return this.renderAnnotationPropertyEditForm()
    var self = this

    var tags = ['<table class="form">']
    tags.push('<tr><th>Name:</th><td>'+formElementFactory({field:'name',editable:true},this)+'</td></tr>')
    tags.push('</table>')
    tags.push('<div><button target="tr" class="foldHandler rotate90">⫸</button><table class="form">')
    tags.push('<tr><th>Location:</th><td><input name="location_path" value="'+this.location_path+'" id="location_path"><button id="chooseTreeNode">Tree Node</button></td></tr>')
    tags.push('<tr><th>Function:</th><td><input name="location_func" value="'+this.location_func+'" id="location_func"><button id="showLocationPolicyEditor">Location Policy Editor</button></td></tr>')
    tags.push('</table></div>')
    tags.push('<div><button target="tr" class="foldHandler rotate90">⫸</button><table class="form">')
    var slots = this.slots;
    for(i=0;i<slots.length;i++) {
        var slot = slots[i]

        // default to open
        //var style = i==0 ? '' : ' style="display:none"'
        var style = ''
        var value = this.sigProper[slot.name]
        var placeholder = null
        if (typeof(value)=='undefined') value = null;
        var input;
        if (slot._writable) {
            if (value===null || value=='') {
                placeholder = ' placeholder="Default"'
                value = ''
            }
            input = '<input class="slot" '+placeholder+' name="'+slot.name+'" value="'+value+'">'
        }
        else{
            if (value===null||value=='') {
                placeholder = ' placeholder="Read Only"'
                value = ''
            }
            input = '<input'+placeholder+' name="'+slot.name+'" value="'+value+'" disabled>'
        }
        tags.push('<tr'+style+'><th>'+slot.name+':</th><td>'+input+'</td></tr>')
    }
    tags.push('</table>')
    tags.push('<button onclick="FBP_deleteBlock()">Delete</button>')
    tags.push('</div>')
    var editForm = document.getElementById('blockPropEditForm')
    editForm.innerHTML = tags.join('')
    sidebarActiveTab('t3')
    $('Button.foldHandler').on('click',function(evt){
        var btn = evt.currentTarget
        var hasOpened = btn.className.indexOf('rotate90')>=0
        var toOpen = !hasOpened
        if (toOpen){
            btn.className='foldHandler rotate90'
        }
        else{
            btn.className = 'foldHandler'
        }
        //pick the next element
        var childNodes = btn.parentNode.querySelectorAll(btn.getAttribute('target'))
        var display = toOpen ? '' : 'none'
        for (var i=1;i<childNodes.length;i++){
            childNodes[i].style.display = display;
        }
    })
    $('#blockPropEditForm input').on('change',function(evt){
        var name = evt.currentTarget.getAttribute('name')
        var isSlot = evt.currentTarget.className.indexOf('slot') >= 0
        if (isSlot){
            for (var i=0,l=self.slots.length;i<l;i++){
                if (self.slots[i].name==name){
                    self.sigProper[name] = evt.currentTarget.value
                    break
                }
            }
        }
        else{
            self[name] = evt.currentTarget.value
            if (name.indexOf('location')>=0) {
                self.location = self.location_path+'#'+self.location_func
            }
            else if (name=='name') {
                document.querySelector('.block-name[blockid="'+self.id+'"]').innerText = self.name
            }
        }
        top.notifyApplicationContentTainted(true)
    })

    $('#chooseTreeNode').on('click',function(e){
        e.stopImmediatePropagation()
        var inputId = 'location_path'
        $.post('/loc_tree', function(data) {
            tree_html = top.generate_tree(data,"locTreeNodeInDialog");

            $('#treeInDialogDiv').empty();
            $('#treeInDialogDiv').html(tree_html);
            $('.locTreeNodeInDialog').click(function () {
                var location_str='';
                var clickedNodeId = parseInt($(this).attr("id"), 10);
                while (clickedNodeId != 0) {
                    location_str = '/' + $("#"+clickedNodeId).text() + location_str;
                    clickedNodeId = Math.floor(clickedNodeId/100);
                }
                $('#selectedTreeNode').val(location_str);
                $('#'+$('#tree_dialog').get(0).dataset.setFor).val(location_str);

                // trigger the onchange event of the location_path element
                var ele = document.getElementById(inputId)
                if ("createEvent" in document) {
                    var evt = document.createEvent("HTMLEvents");
                    evt.initEvent("change", false, true);
                    ele.dispatchEvent(evt);
                }
                else
                    ele.fireEvent("onchange");
            });
            $('#confirm_tree_dialog').click(function(){
                $('#'+$('#tree_dialog').get(0).dataset.setFor).val($('#selectedTreeNode').val());
            });
            $('.dialogCloseButton').click(function() {
                $('#tree_dialog').dialog("close");
            });
        });
        $('#display').treeview({
            collapsed: true,
            animated: "fast",
        });
        $('#tree_dialog').get(0).dataset.setFor = inputId;

        $('#tree_dialog').dialog();
        //$('#tree_dialog').draggable();
        $('#tree_dialog').show();

        //HY: adjust the position of the dialog
        var viewport = getViewportSize()
        setTimeout(function(){
            var dialog = $('#tree_dialog').parent()
            var height = Math.min(dialog.height(),viewport.height-50)
            dialog.find('.ui-dialog-content').css('height',height-40)
            var left = (viewport.width-dialog.width())*0.5
            var top = Math.max(0,(viewport.height-dialog.height())*0.5)
            dialog.css({left:left+'px',top:top+'px',height:height+'px'})
            //HY: also, make the input field longer
            $('#selectedTreeNode').css('width','100%');
        },100)
    })
    $('#showLocationPolicyEditor').on('click',function(){
        var modalFrame = document.getElementById('locationpolicyeditor')
        var editor = new LocationPolicyEditor(modalFrame,modalFrame.querySelector('.modal-dialog-content'));
        editor.setPolicy(editForm.querySelector('input#location_func').value)
        editor.render()
        editor.policyChangeListener = function(policy){
            self.location_func = policy
            self.location = self.location_path+'#'+self.location_func
            editForm.querySelector('input#location_func').value = policy
            top.notifyApplicationContentTainted(true)
        }
    })
}

Block.prototype.renderAnnotationPropertyEditForm = function(){
    var self = this
    var tags = ['<table class="form">']
    tags.push('<tr><td><textarea style="width:85%;height:150px">')
    var value = this.sigProper['content']
    tags.push((value ? decodeURIComponent(value) : '')+'</textarea></td></tr>')
    tags.push('</table><hr>')
    tags.push('<button onclick="FBP_deleteBlock()">Delete</button>')
    var editForm = document.getElementById('blockPropEditForm')
    editForm.innerHTML = tags.join('')
    var textarea = editForm.querySelector('textarea')
    var updateValue = function(){
        self.div.find('div.content').text(textarea.value)
        self.sigProper['content'] = encodeURIComponent(textarea.value)
        top.notifyApplicationContentTainted(true)
    }
    var updateTimer;
    textarea.onkeypress=function(evt){
        if (updateTimer) clearTimeout(updateTimer)
        updateTimer = setTimeout(updateValue,1000)
    }
    textarea.onchange=function(evt){
        updateValue()
    }
    sidebarActiveTab('t3')
}
Block.hitTest=function(x,y) {
    var i;
    for(i=0;i<Block.widgets.length;i++) {
        var bounds = Block.widgets[i].getBounds();
        //console.log(bounds)
        if(x >= bounds.left && x <= bounds.right && y >= bounds.top && y <= bounds.bottom){
            return Block.widgets[i];
        }
    }
    return null;
}

$(document).ready(function() {
    /*
    Disabled by HY
    Because the propertyEditor has been moved to sidebar
    //$('#propertyeditor').dialog({autoOpen:false});
    */
});
