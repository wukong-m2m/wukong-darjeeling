/*
Author: Hsin Yuan Yeh
Data: Apr 18, 2015
Dependency: hyutil.js
*/

function WuKongApplication(srcobj){
    /*  confdata is requested by /application/<id>
        by wkpf.globals.applications[app_ind].config()
    */
    var attrs = ['app_name','desc','dir','id','version','xml','disabled']
    copyProperties(srcobj,this,attrs)
    //default values
    //if (!this.desc) this.desc = 'this is sample text of description of this application'
}
WuKongApplication.prototype = {
    updateAttributes:function(srcobj){
        //console.log(srcobj)
        var attrs = ['app_id','default_location','logs','node_infos','set_location']
        copyProperties(srcobj,this,attrs)
    },
    /* for FBP editor */
    renderForSidebar:function(el){
        var tags = ['<table class="application-property">']
        //[caption, attr-name , editable, tag-type ]
        var fields = [
            {title:'Name',field:'app_name',editable:true},
            {title:'id',field:'id',editable:false},
            {title:'',field:'desc',editable:true,tag:'textarea',style:'height:100px;width:80%;border:none;'},
            ];
        for (var i in fields){
            var title = fields[i].title
            if (title)
                tags.push('<tr><th>'+title+':</th><td>')
            else
                tags.push('<tr><td colspan="2">')
            tags.push(formElementFactory(fields[i],this)+'</td></tr>')
        }
        tags.push('</table>')
        el.innerHTML = tags.join('')
        var self = this
        document.querySelector('input[name="app_name"]').onchange = function(evt){
            self.app_name = evt.currentTarget.value
            top.notifyApplicationMetaTainted(true)
        }
        document.querySelector('textarea[name="desc"]').onchange = function(evt){
            self.desc = evt.currentTarget.value
            top.notifyApplicationContentTainted(true)
        }
    },
}
/* temporary solution to assign category for block (wuClass) */
function registerWuClassCategory(block){
	var c = WuClassCategory.lookupTable[block.typename]
	if (!c) {
	    c = WuClassCategory.lookupTable['Generic']
	    console.log('Can not find category of "'+block.typename+'"')
	}
	c.members.push(block);
	block.category = c;
	return
}
/* construct the category tree */
//HY:"wuClassCategoaryTableRaw" is created by generate_categorytable_4_js.py
var wuClassCategoaryTableRaw = {"Actuator": {"UART": ["Grove_MP3", "Gesture_MP3"], "Digital Output": ["Light_Actuator", "LED", "Fan", "Relay"], "Analog Output": ["RGBLED", "Dimmer", "Mist"], "PWM": ["Buzzer", "Sound", "MOSFET_LED"], "I2C": ["Grove_LCD"]}, "Sensor": {"Digital Input": ["PIR_Sensor", "Binary_Sensor", "Magnetic_Sensor", "Button", "Touch_Sensor"], "UART": ["User", "Gesture"], "Others": ["Gh_Sensor"], "Fast Digital I/O": ["Ultrasound_Sensor"], "Unit Test": ["Binary_TestSensor", "Integer_TestSensor"], "Analog Input": ["Light_Sensor", "Slider", "Ir_Sensor", "Microphone_Sensor", "Pressure_Sensor_0", "Temperature_Sensor", "Sound_Sensor"], "I2C": ["Temperature_Humidity_Sensor"]}, "Software": {"Calculation": ["Math_Op"], "Special Usage": ["Server", "Multiplexer", "Virtual_Slider", "Controller", "User_Aware", "Plugin"], "Condition": ["If_Short", "If_Boolean", "Condition_Selector_Boolean", "Condition_Selector_Short"], "Logic": ["Threshold", "And_Gate", "Or_Gate", "Xor_Gate", "Not_Gate", "Equal"]}}

function WuClassCategory(title,parentCategory){
    this.title = title
    this.parent = parentCategory;
    this.children = [] //sub category
    if (parentCategory) parentCategory.children.push(this)
    this.members = [] // member WuClass
}
WuClassCategory.lookupTable = {}
WuClassCategory.rootCategory = new WuClassCategory('Root',null)
for (var level0 in wuClassCategoaryTableRaw){
    var c0 = new WuClassCategory(level0,WuClassCategory.rootCategory)
    for (var level1 in wuClassCategoaryTableRaw[level0]){
        var c1 = new WuClassCategory(level1,c0)
        for (var i=0,l=wuClassCategoaryTableRaw[level0][level1].length;i<l;i++){
            var level3 = wuClassCategoaryTableRaw[level0][level1][i];
            WuClassCategory.lookupTable[level3] = c1;
        }
    }
}
//category for "not found"
var generic = new WuClassCategory('Generic',WuClassCategory.rootCategory)
WuClassCategory.lookupTable['Generic'] = new WuClassCategory('Unclassified',generic)
