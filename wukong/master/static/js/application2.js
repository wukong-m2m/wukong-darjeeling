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
                alert(data.mesg);
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
        app_name = prompt('Please enter the application name:', 'New Application')
        if(app_name != '' && app_name != null) {
          $.post('/applications/new', {app_name: app_name}, function(data) {
            if (data.status == '1') {
                alert(data.mesg);
            }
            else {
                console.log(data);
                application_fill();
            }
          });
        }
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
                alert(msg)
            }
            r.onload = function(){
                var content = r.result
                var xmlEle = document.createElement('xml')
                xmlEle.innerHTML = content
                var app_nameEle = xmlEle.querySelector('app_name')
                if (app_nameEle) {
                    /*
                     * The <app_name></app_name> is added when this file is downloaded.
                     * Now, we remove it before sendback to server.
                     */
                    app_nameEle.parentNode.removeChild(app_nameEle)
                }
                else{
                    return uploadError('Format Error')
                }
                // set default to be disabled application
                var appEle = xmlEle.querySelector('application')
                var disabledEle = document.createElement('disabled')
                disabledEle.innerText = 1
                appEle.insertBefore(disabledEle,appEle.firstChild)

                content = xmlEle.innerHTML
                //console.log('----update----')
                //console.log(content)

                var app_name = app_nameEle.innerText
                if(app_name != '') {
                  $.post('/applications/new', {app_name: app_name,xml:content}, function(data) {
                    if (data.status == '1') {
                        alert(data.mesg);
                    }
                    else {
                        application_fill();
                    }
                    dialog.parentNode.removeChild(dialog)
                  });
                }
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
            var yes = confirm('Are you sure to delete '+app.app_name+'?')
            if (!yes) return
            var app_id = $(this).data('app_id');
            $.ajax({
                type: 'delete',
                url: '/applications/' + app_id,
                success: function(data) {
                    if (data.status == 1) {
                        alert(data.mesg);
                    }

                    application_fill();
                }
            });
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
                    alert(data.mesg);
                    application_fill();
                } else {
                    //topbar = data.topbar;
                    var viewport = updateCanvasHeight()
                    var querystring = '?vw='+viewport.width+'&vh='+viewport.height
                    $.get('/applications/' + app_id + '/deploy'+querystring, function(data) {
                        if (data.status == 1) {
                            alert(data.mesg);
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
    console.log('polling');
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


