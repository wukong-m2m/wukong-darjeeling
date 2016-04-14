// vim: ts=4 sw=4

global_polling_status = false;

function polling_is_stopped() {
    return !global_polling_status
}

$(document).ready(function() {
    init();
});

function init()
{
    window.options = {repeat: false};

    // Give user a chance to save the tainted application.
    // This should be here before all other click handlers
    var triggers = document.querySelectorAll('.navbar-inner a')
    for (var i=0;i<triggers.length;i++){
        triggers[i].onclick = function(evt){
            var pass = top.notifyApplicationContentChanged()
            if (!pass) {
                evt.preventDefault()
                evt.stopImmediatePropagation()
            }
        }
    }

    // Top bar
    $('#application').click(function() {
        $('#node-editor').parent().removeClass('active');
        $('#application').parent().addClass('active');
        $('#locationTree').parent().removeClass('active');
        $('#designer').parent().removeClass('active');
        window.options.repeat = false;
        application_fill();
    });

    $('#node-editor').click(function() {
        $('#node-editor').parent().addClass('active');
        $('#application').parent().removeClass('active');
        $('#locationTree').parent().removeClass('active');
        $('#designer').parent().removeClass('active');
        window.options.repeat = false;
        $('#content').html('<H1>Please wait, the node editor is loading now...</H1>');
        $.get('/testrtt', function(data) {
            if (data.status == '1') {
                myAlert(data.mesg);
            }
            else {
                $('#content').html(data.testrtt);
            }
        });
    });
    $('#open-app-store').click(function(){
        var viewportSize = getViewportSize()
        var dialog = document.createElement('div')
        dialog.className = 'modal-dialog appstore'
        var style = dialog.style
        style.display = 'inline-block'
        style.position = 'absolute';
        style.left = viewportSize.width * 0.035 + 'px'
        style.top = viewportSize.height * 0.02 + 'px'
        style.width = viewportSize.width * 0.90 + 'px'
        style.height = viewportSize.height * 0.80 + 'px'
        document.body.insertBefore(dialog,document.querySelector('.navbar'))

        var closeStore = function(){
            dialog.parentNode.removeChild(dialog)
        }
        // load content
        var appStoreUrl = '/static/appstore/index.html'
        $(dialog).load(appStoreUrl,function(){
            appStoreInit(closeStore)
            var closeBtnDiv = document.createElement('div')
            closeBtnDiv.style.position = 'absolute'
            closeBtnDiv.style.textAlign = 'center'
            closeBtnDiv.style.width = '100%'
            closeBtnDiv.style.top = (parseInt(style.height)-20) + 'px'
            closeBtnDiv.innerHTML = '<button class="blue">Close</button>'
            dialog.appendChild(closeBtnDiv)
            closeBtnDiv.querySelector('button').onclick = closeStore
        })
    })
    $('#mptn-gateway').html('&lt;'+location.hostname+'&gt;:1')
    $('#open-gateway').click(function(){
        var viewportSize = getViewportSize()
        var dialog = document.createElement('div')
        dialog.className = 'modal-dialog appstore'
        var style = dialog.style
        style.display = 'inline-block'
        style.position = 'absolute';
        style.left = viewportSize.width * 0.035 + 'px'
        style.top = viewportSize.height * 0.02 + 'px'
        style.width = viewportSize.width * 0.90 + 'px'
        style.height = viewportSize.height * 0.80 + 'px'
        document.body.insertBefore(dialog,document.querySelector('.navbar'))

        var closeStore = function(){
            dialog.parentNode.removeChild(dialog)
        }
        // load content
        var appStoreUrl = '/static/gateway/index.html'
        $(dialog).load(appStoreUrl,function(){
            MPTNGatewayInit(closeStore)
            var closeBtnDiv = document.createElement('div')
            closeBtnDiv.style.position = 'absolute'
            closeBtnDiv.style.textAlign = 'center'
            closeBtnDiv.style.width = '100%'
            closeBtnDiv.style.top = (parseInt(style.height)-20) + 'px'
            closeBtnDiv.innerHTML = '<button class="blue">Cancel</button>'
            dialog.appendChild(closeBtnDiv)
            closeBtnDiv.querySelector('button').onclick = closeStore
        })
    })
    $('#open-javaeditor').click(function(){
        var viewportSize = getViewportSize()
        var dialog = document.createElement('div')
        dialog.className = 'modal-dialog appstore'
        var style = dialog.style
        style.display = 'inline-block'
        style.position = 'absolute';
        style.left = viewportSize.width * 0.035 + 'px'
        style.top = viewportSize.height * 0.02 + 'px'
        style.width = viewportSize.width * 0.90 + 'px'
        style.height = viewportSize.height * 0.80 + 'px'
        document.body.insertBefore(dialog,document.querySelector('.navbar'))

        var closeStore = function(){
            dialog.parentNode.removeChild(dialog)
        }
        // load content
        var appStoreUrl = '/static/javaeditor/index.html'
        $(dialog).load(appStoreUrl,function(){
            JavaEditorInit(closeStore)
            var closeBtnDiv = document.createElement('div')
            closeBtnDiv.style.position = 'absolute'
            closeBtnDiv.style.textAlign = 'center'
            closeBtnDiv.style.width = '100%'
            closeBtnDiv.style.top = (parseInt(style.height)-20) + 'px'
            closeBtnDiv.innerHTML = '<button class="blue compile-javaeditor">Compile</button><button style="margin-left:20px" class="close-javaeditor">Close</button>'
            dialog.appendChild(closeBtnDiv)
            closeBtnDiv.querySelector('button.close-javaeditor').onclick = closeStore
            closeBtnDiv.querySelector('button.compile-javaeditor').onclick = function(){
                compileJavaWuClassScript()
            }
        })
    })
    /*
    $('#locationTree').click(function() {
        $('#node-editor').parent().removeClass('active');
        $('#application').parent().removeClass('active');
        $('#locationTree').parent().addClass('active');
        $('#designer').parent().removeClass('active');
        window.options.repeat = false;
        $.post('/loc_tree', function(data) {
                display_tree("#content",data);
                $('#content').append(data.node);
                document.body.dataset.installer_mode = "false";
        });
    });
    */
    /*
    $('#designer').click(function() {
        $('#node-editor').parent().removeClass('active');
        $('#application').parent().removeClass('active');
        $('#locationTree').parent().removeClass('active');
        $('#designer').parent().addClass('active');
        window.options.repeat = false;
        $('#content').html('<iframe width=100% height=100% src=/ide></iframe>');
    });
    */
    application_fill();

}

function application_fill()
{
    $('#content').empty();
    $('#content').append($('<p><button id="appadd" class="btn btn-primary">Create New</button><button style="margin-left:10px" id="appupload" class="btn">Upload/Restore</button></p>'));
    $.ajax({
        url: '/applications',
        type: 'POST',
        dataType: 'json',
        success: function(r) {
            application_fillList(r);
        },
        error: function(xhr, textStatus, errorThrown) {
            console.log(errorThrown);
        }
    });

    $('#appadd').click(function() {
        app_name = myPrompt('Create New Application:', {placeholder:'name of this application'},function(app_name){
            if(app_name != '' && app_name != null) {
              $.post('/applications/new', {app_name: app_name}, function(data) {
                if (data.status == '1') {
                    myAlert(data.mesg);
                }
                else {
                    console.log(data);
                    application_fill();
                }
              });
            }
        })
    });
    $('#appupload').click(function() {
        var viewportSize = getViewportSize()
        var dialog = document.createElement('div')
        dialog.className = 'modal-dialog uploadform'
        //position the dialog
        var style = dialog.style
        style.display = 'inline-block'
        style.position = 'absolute';
        var width = 400
        var height = 200
        style.left = (viewportSize.width-width) /2 + 'px'
        style.top = (viewportSize.height - height)/2 + 'px'
        style.width = width+'px'
        style.height = height+'px'
        document.body.insertBefore(dialog,document.querySelector('.navbar'))

        //create the upload form in the dialog
        var formHTML = '<div style="padding:30px"><p>WuKong Application XML:</p><form style="margin-top:10px"><input name="file" style="line-height:20px;zoom:120%;height:40px;color:blue;" type="file"></form><div style="text-align:center;margin-top:50px"><button id="uploadbtn" class="my-button-blue" style="margin-right:20px">Upload</button><button class="my-button" id="uploadclosebtn">Cancel</button></div></div>'
		dialog.innerHTML = formHTML;
        // load content
        //var appStoreUrl = '/static/appstore/index.html'
        //$(dialog).load(appStoreUrl)
        dialog.querySelector('#uploadclosebtn').onclick = function(){
            dialog.parentNode.removeChild(dialog)
        }
        dialog.querySelector('#uploadbtn').onclick = function(){
            var form = dialog.querySelector('form')
            var formData = new FormData(form);
            //var url = '/applications/'+id+'/fbp/upload'
            var f = form.querySelector('input').files[0]
            var r = new FileReader();
            var uploadError = function(msg){
                dialog.parentNode.removeChild(dialog)
                myAlert('Upload Failure',msg)
            }
            r.onload = function(){
                var content = r.result
                var app_name = content.match(/<app_name>(.+)<\/app_name>/)[1]
                /*
                 * The <app_name></app_name> is added when this file is downloaded.
                 * Now, we remove it before sendback to server.
                 */
                content = content.replace(/<app_name>.+<\/app_name>/,'<disabled>1</disabled>')
                $.post('/applications/new', {app_name: app_name,xml:content}, function(data) {
                    if (data.status == '1') {
                        myAlert('Upload Failure',data.mesg);
                    }
                    else {
                        application_fill();
                    }
                    dialog.parentNode.removeChild(dialog)
                });
            }
            r.readAsText(f)
            /*
            $.ajax({
                url: url,
                type: 'POST',
                xhr: function() {  // custom xhr
                    myXhr = $.ajaxSettings.xhr();
                    if(myXhr.upload){ // if upload property exists
                        myXhr.upload.addEventListener('progress', progressHandlingFunction, false); // progressbar
                    }
                    return myXhr;
                },
                //Ajax events
                success: completeHandler = function(data) {
                    if(navigator.userAgent.indexOf('Chrome')) {
                        var catchFile = $(":file").val().replace(/C:\\fakepath\\/i, '');
                    }
                    else {
                        var catchFile = $(":file").val();
                    }
                    var writeFile = $(":file");
                    writeFile.html(writer(catchFile));
                    $("*setIdOfImageInHiddenInput*").val(data.logo_id);
                },
                error: errorHandler = function() {
                    alert("Något gick fel");
                },
                // Form data
                data: formData,
                //Options to tell JQuery not to process data or worry about content-type
                cache: false,
                contentType: false,
                processData: false
            }, 'json');
            */
        }

    });
}

function application_fillList(r)
{
    var i;
    var len = r.length;
    var m = $('#content');

    applist = $('<table id=applist></table>');
    var removeHandler = function() {
            var app_id = $(this).data('app_id');
            var app = null;
            for (var i=0,_app;_app=r[i];i++){
                if (_app.id==app_id){
                    app = _app;
                    break;
                }
            }
            var self = this
            var yes = myConfirm('Confirm Deletion','Are you sure to delete '+app.app_name+'?',function(yes){
                if (!yes) return;
                var app_id = $(self).data('app_id');
                $.ajax({
                    type: 'delete',
                    url: '/applications/' + app_id,
                    success: function(data) {
                        if (data.status == 1) {
                            myAlert('Deletion Failure',data.mesg);
                        }

                        application_fill();
                    }
                });
            })
        }

    var nameHandler = function() {
            var topbar;
            var app_id = $(this).data('app_id');

            $('#content').empty();
            $('#content').block({
                message: '<h1>Processing</h1>',
                css: { border: '3px solid #a00' }
            });
            $.post('/applications/' + app_id, function(data) {
                if (data.status == 1) {
                    myAlert('Application Error',data.mesg);
                    application_fill();
                } else {
                    //topbar = data.topbar;
                    var viewport = updateCanvasHeight()
                    var querystring = '?vw='+viewport.width+'&vh='+viewport.height
                    $.get('/applications/' + app_id + '/deploy'+querystring, function(data) {
                        if (data.status == 1) {
                            myAlert('Depoly Failure',data.mesg);
                            application_fill();
                        } else {
                            // injecting script to create application interface
                            page = $(data.page);
                            $('#content').html(page)

                            // this would make the canvas scrollable
                            updateCanvasHeight()

                            $('#content').unblock();

                            // Application polling will be optional
                        }
                    });
                }
            });
        }

    for(i=0; i<len; i++) {
        var app = r[i]
        // Html elements
        var appentry = $('<tr class="listitem"></tr>');
        var name = $('<td class="appname" data-app_id="' + app.id + '" id="appname'+i+'"><b style="color:'+(app.disabled ? '#0c0c0c' : '#000')+'">' +(app.disabled ? '◯ ':'◉ ')+'</b>'+app.app_name + '</td>');
        var act = $('<td class=appact id=appact'+i+'></td>');

        var remove = $('<button class="close" data-app_id="' +app.id + '" id=appdel'+i+'>&times;</button>');

        // Enter application
        name.click(nameHandler);

        remove.click(removeHandler);

        act.append(remove);

        appentry.append(name);
        appentry.append(act);
        applist.append(appentry);
        //application_setupButtons(i, r[i].id);
    }

    m.append(applist);
}

function start_polling()
{
    // start polling
    window.options = {repeat: true};
}

function stop_polling()
{
    // stop polling
    window.options.repeat = false;
}

function application_polling(app_id, destination, property, callback)
{
    // stops previous polling
    stop_polling();

    while (!polling_is_stopped()) {};

    // starts a new one
    start_polling();

    // sets default destination
    if (typeof destination == 'undefined') {
        destination = '#mapping-progress';
    }

    // sets default property
    if (typeof property == 'undefined') {
        property = 'all_wukong_status';
    }

    poll('/applications/' + app_id + '/poll', 0, window.options, function(data) {
        //data.wukong_status = data.wukong_status.trim();
        //data.application_status = data.application_status.trim();
        callback(data)
        /*
        $(destination).empty();
        for (var i=0; i<data[property].length; i++) {
            $(destination).append("<pre>[" + data[property][i].level + "] " + data[property][i].msg + "</pre>");
        }
        */
        //if (data.wukong_status === "close" || data.application_status === "close") {
            //$('#deploy_results').dialog('close');
        //} else if (!(data.wukong_status === "" && data.application_status === "")) {
            //$('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
            //$('#deploy_results #wukong_status').text(data.wukong_status);
            //$('#deploy_results #application_status').text(data.application_status);
        //}
    });
}

function content_scaffolding(topbar, editor)
{
    $('#content').append(topbar);
    $('#content').append(editor);
}


/*
function application_setupButtons(i, id)
{
    $('#appmonitor'+i).click(function() {
        alert("not implemented yet");
    });
    $('#appdeploy'+i).click(function() {
        $.get('/applications/' + id + '/deploy', function(data) {
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                deploy_show(data.page, id);
            }
        });
    });
    $('#appdel'+i).click(function() {
        $.ajax({
            type: 'delete',
            url: '/applications/' + id,
            success: function(data) {
                if (data.status == 1) {
                    alert(data.mesg);
                }

                application_fill();
            }
        });
    });
}
*/

/*
function application_setupLink(app_id)
{
    $.post('/applications/' + app_id, function(data) {
        // create application
        $('#content').html('<div id="topbar"></div><iframe width="100%" height="100%" src="/applications/' + app_id + '/fbp/load"></iframe>');
        $('#topbar').append(data.topbar);
        $('#topbar #back').click(function() {
            application_fill();
        });
    });
}
*/

/*
function deploy_show(page, id)
{
    // deployment page
    $('#content').html(page);
    $('#content #back').click(function() {
        application_fill();
    });

    $('#content #deploy').click(function() {
        if ($('#content input').length == 0) {
            alert('The master cannot detect any nearby deployable nodes. Please move them in range of the basestation and try again.');
        } else if ($('#content input[type="checkbox"]:checked').length == 0) {
            alert('Please select at least one node');
        } else {
            var nodes = new Array();
            nodes = _.map($('#content input[type="checkbox"]:checked'), function(elem) {
                return parseInt($(elem).val(), 10);
            });

            console.log(nodes);

            $.post('/applications/' + id + '/deploy', {selected_node_ids: nodes}, function(data) {
                deploy_poll(id, data.version);
            });
        }
    });
}
*/

/*
function deploy_poll(id, version)
{
    $.post('/applications/'+id+'/deploy/poll', {version: version}, function(data) {
        $('#progress #compile_status').html('<p>' + data.deploy_status + '</p>');
        $('#progress #normal').html('<h2>NORMAL</h2><pre>' + data.normal.join('\n') + '</pre>');
        $('#progress #urgent_error').html('<h2>URGENT</h2><pre>' + data.error.urgent.join('\n') + '</pre>');
        $('#progress #critical_error').html('<h2>CRITICAL</h2><pre>' + data.error.critical.join('\n') + '</pre>');

        if (data.compile_status < 0) {
            deploy_poll(id, data.version);
        }
        else {
            $('#progress').dialog({buttons: {Ok: function() {
                $(this).dialog('close');
            }}})
        }
    });
}
*/

// might have to worry about multiple calls :P
function poll(url, version, options, callback)
{
    var forceRepeat = false;
    if (typeof options != 'undefined') {
        // repeat is an object with one key 'repeat', or it will be pass-by-value and therefore can't be stopped
        forceRepeat = options.repeat;
    }

    global_polling_status = true;
    $.post(url, {version: version}, function(data) {
        if (typeof callback != 'undefined') {
            callback(data);
        }

        if (data && data.ops && data.ops.indexOf('c') != -1) {
            stop_polling();
        } else if (forceRepeat) {
            setTimeout(function() {
                poll(url, data.version, options, callback);
            }, 1000);
        }

        global_polling_status = false;
    });
}

function showNodeRedFrame(show){
    if (typeof(show)=='undefined') show = true
    var div = document.getElementById('node-red-frame')
    var iframe = div.querySelector('iframe')
    if (show){
        iframe.src = location.protocol+'//'+location.hostname+':1880'
        var size = getViewportSize();
        div.style.display=''
        div.style.width = (size.width-1)+'px'
        div.style.height = (size.height-1)+'px'
    }
    else{
        iframe.src = 'about:blank'
        div.style.display='none'
        div.style.width = '0px'
        div.style.height = '0px'
    }
}

/* HY:(2016/1/1)
 * Customized alert,prompt and confirm to replace browser defaults.
 * By the caution about their problme reported by YC.
 */
function moveDialogToCenter(){
    var viewportSize = getViewportSize()
    var dialogRect = dialog.getBoundingClientRect()
    dialog.style.left = Math.round((viewportSize.width-dialogRect.width)/2)+'px'
    dialog.style.top =  Math.round((viewportSize.height-dialogRect.height)/2)+'px'
}
function myAlert(title,mesg,callback,options){
    var dialog = document.getElementById('dialog')
    dialog.querySelector('.title').style.display='block';
    dialog.querySelector('.title').innerHTML = title
    dialog.querySelector('.content').innerHTML = mesg
    var buttons = []
    var extraButtonNames = []
    if (options && options.buttons){
        for (var i in options.buttons){
            var btn = options.buttons[i]
            buttons.push('<button name="'+btn.name+'" class="'+(btn.className || '')+'">'+btn.title+'</button>')
            extraButtonNames.push('button[name="'+btn.name+'"]')
        }
    }
    else{
        buttons.push('<button name="ok" class="ok my-button-blue">OK</button>')
    }
    dialog.querySelector('.buttons').innerHTML = buttons.join('')
    dialog.querySelector('.x').onclick= function(){
        dialog.style.display='none'
        if (callback) callback(null)
    }
    dialog.querySelector('.ok').onclick= function(){
        dialog.style.display='none'
        if (callback) callback(null)
    }
    if (extraButtonNames.length){
        dialog.querySelectorAll(extraButtonNames.join(',')).forEach(function(ele){
            ele.onclick = function(){
                dialog.style.display='none'
                if (callback) callback(ele.getAttribute('name'))
            }
        })
    }
    dialog.style.display = 'block'
    dialog.style.height = '150px'
    dialog.style.opacity = 1;

    moveDialogToCenter()
}
function myPrompt(title,options,callback){
    var dialog = document.getElementById('dialog')
    dialog.querySelector('.title').innerHTML = title;
    dialog.querySelector('.content').innerHTML = '<input style="width:100%;height:40px;font-size:1.5em">'
    var input = dialog.querySelector('.content input')
    if (options.placeholder) input.setAttribute('placeholder',options.placeholder)
    if (options.value) input.value = options.value
    var buttons = [
        '<button class="ok my-button-blue">OK</button>',
        '<button class="cancel">Cancel</button>',
        ]
    dialog.querySelector('.buttons').innerHTML = buttons.join('')
    dialog.querySelector('.x').onclick = function(){
            dialog.style.display='none'
            callback(null)
    }
    dialog.querySelector('.cancel').onclick = function(){
            dialog.style.display='none'
            callback(null)
    }
    dialog.querySelector('.ok').onclick = function(){
        dialog.style.display='none'
        var text = dialog.querySelector('input').value
        callback(text)
    }
    dialog.style.display = 'block'
    dialog.style.height = '150px'

    moveDialogToCenter()
}
function myConfirm(title,content,callback,blueCancelButton){
    var dialog = document.getElementById('dialog')
    dialog.querySelector('.title').innerHTML = title;
    dialog.querySelector('.content').innerHTML = content

    var input = dialog.querySelector('.content input')
    var buttons = [
        '<button class="ok'+(blueCancelButton ? '' : ' my-button-blue')+'">YES</button>',
        '<button class="cancel'+(blueCancelButton ? ' my-button-blue':'')+'">Cancel</button>',
        ]
    dialog.querySelector('.buttons').innerHTML = buttons.join('')
    dialog.querySelector('.x').onclick = function(){
            dialog.style.display='none'
            callback(null)
    }
    dialog.querySelector('.cancel').onclick = function(){
            dialog.style.display='none'
            callback(null)
    }
    dialog.querySelector('.ok').onclick = function(){
        dialog.style.display='none'
        callback(true)
    }
    dialog.style.display = 'block'
    dialog.style.height = '150px'

    moveDialogToCenter()

}