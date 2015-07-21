/* Expose global ShiftKeyDown */
var ShiftKeyDown = false;
var EscKeyUp = false;
function watchingSpecialKeys(){
    $(document).on('keyup keydown', function(e){
        ShiftKeyDown = e.shiftKey
        if (Block.current){
            if (ShiftKeyDown) $(Block.current.div).find('.block-signals').addClass('shiftKey')
            else $(Block.current.div).find('.block-signals').removeClass('shiftKey')
        }
    });
}
function copyProperties(src,dst,attrs){
    for (var i in attrs){
        dst[attrs[i]] = src[attrs[i]]
    }
}
function formElementFactory(fieldmeta,valueSource){
    var value = valueSource[fieldmeta.field]
    if (value===null) value = ''
    else if (typeof(value)=='undefined') value = ''
    else if (typeof(value)=='string'){
        value = value.replace(/["<>]/g,'') //no injection
    }
    else value = ''+value //cast to string
    if (!fieldmeta.editable) return value;
    var tagType = fieldmeta.tag || 'input'
    var tag;
	var name = fieldmeta.name || fieldmeta.field
    switch(tagType.toLowerCase()){
    	case 'input':
    		var style = fieldmeta.style || 'width:100%'
    		var disabled = fieldmeta.disabled ? ' disabled' : ''
    		tag = '<input type="text" name="'+name+'" value="'+value+'" style="'+style+'"'+disabled+'>'
    		break;
    	case 'textarea':
    		var style = fieldmeta.style || 'width:100%;height:100px';
    		tag = '<textarea name="'+name+'" style="'+style+'">'+value+'</textarea>'
    		break;
    }
    return tag
}

function getViewportSize(){
    if (typeof(_viewportSize)!='undefined'){
        return _viewportSize;
    }
    var w = Math.max(document.documentElement.clientWidth, window.innerWidth || 0)
    var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0)
    return {width:w,height:h}
}
function getViewportPosition(){
    if ($('#canvasandclient').length)
        return $('#canvasandclient').position()
    else {
        var vs = getViewportSize()
        return {left:-vs.width,top:-vs.height}
    }
}
function getViewportOffset(){
    if ($('#canvasandclient').length)
        return $('#canvasandclient').offset()
    else {
        var vs = getViewportSize()
        return {left:-vs.width,top:-vs.height}
    }
}
function getAbsoluteHeight(el) {
  el = (typeof el === 'string') ? document.querySelector(el) : el;
  var styles = window.getComputedStyle(el);
  var margin = parseFloat(styles['marginTop']) +
               parseFloat(styles['marginBottom']);
  return Math.ceil(el.offsetHeight + margin);
}
function updateCanvasHeight(){
    var viewport = getViewportSize();
    //console.log('---updateCanvasHeight()---')
    //console.log(viewport)
    var navbar = document.querySelector('.navbar')
    var canvasHeight = viewport.height
    if (navbar){
        //in application2.html
        var navBarHeight = getAbsoluteHeight(navbar)
        var bottomHeight = 20;
        canvasHeight = viewport.height - navBarHeight - bottomHeight;
        var canvas = document.querySelector('.contentBox')
        canvas.style.height = canvasHeight+'px'
    }
    //check if the FBP editor is activated
    var fbpiframe = document.querySelector('#fbp')
    if (fbpiframe){
        // in deployment2.html
        fbpiframe.style.height = canvasHeight+'px'
        document.querySelector('iframe').style.height = (canvasHeight-5)+'px'
    }
    /*
    var toolbar = document.querySelector('#toolbar')
    if (toolbar){
        // in fbp2.html
        toolbar.style.width = viewport.width+'px'
    }
    */
    return {width:viewport.width,height:canvasHeight}
}

var loadingCount = 0;
function loading(yes){
  var title;
  if (yes) title = (typeof(yes)=='string') ? yes : 'Working'
  if ((loadingCount<0 && !yes) || (loadingCount>0 && yes)) {
    if (title) document.getElementById('loading-text').innerHTML = title;
    loadingCount += yes ? 1 : -1;
    return;
  }
  loadingCount += yes ? 1 : -1;
  if (loadingCount==1){
    // start loading
    document.getElementById('loading-text').innerHTML = title;
    document.getElementById('loading').style.display='inline-block';
    document.getElementById('loading').style.top = (window.pageYOffset+50)+'px'
    if (document.querySelector('#canvasbox')){
        document.querySelector('#canvasbox').style.opacity = 0.25;
        document.querySelector('#sidebar').style.opacity = 0.25;
    }
  }
  else{
    // stop loading
    document.getElementById('loading').style.display='none';
    if (document.querySelector('#canvasbox')){
        document.querySelector('#canvasbox').style.opacity = 1;
        document.querySelector('#sidebar').style.opacity = 1;
    }
  }
}
function stopLoading(){loading(false)}

// application content in FBP
var _AppContentTainted_ = false;
function notifyApplicationContentTainted(yes){
    _AppContentTainted_ = yes
}
// app_name etc
var _AppMetaTainted_ = false;
function notifyApplicationMetaTainted(yes){
    _AppMetaTainted_ = yes
}
// Warning user to save if necessary
function notifyApplicationContentChanged(){
    if (!(_AppContentTainted_||_AppMetaTainted_)) return true
    var yes = confirm('Changes not been saved, Are you sure ?')
    if (yes){
        _AppContentTainted_ = false
        _AppMetaTainted_ = false
    }
    return yes
}