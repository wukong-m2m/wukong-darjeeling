global_polling_status = false;

function polling_is_stopped() {
    return !global_polling_status
}

function start_polling()
{
    // start polling
    window.options = {repeat: true};
}

function stop_polling()
{
    // stop polling
    window.options = {repeat: false};
}

function application_polling(app_id, destination, property)
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
        console.log(data);
        $(destination).empty();
        for (var i=0; i<data[property].length; i++) {
            $(destination).append("<pre>[" + data[property][i].level + "] " + data[property][i].msg + "</pre>");
        }
        
        //if (data.wukong_status === "close" || data.application_status === "close") {
            //$('#deploy_results').dialog('close');
        //} else if (!(data.wukong_status === "" && data.application_status === "")) {
            //$('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open');
            //$('#deploy_results #wukong_status').text(data.wukong_status);
            //$('#deploy_results #application_status').text(data.application_status);
        //}
    });
}

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

$(function() {
    $('#deployment-tab').click(function () {
        $('a#nodes-btn').click();
    });
    $('a#nodes-btn').click(function(e) {
        console.log('refresh nodes');
        $(this).tab('show');

        $('#nodes').block({
            message: '<h1>Processing</h1>',
            css: { border: '3px solid #a00' }
        });
        $.post('/nodes/refresh', function(data) {
            $('#nodes').html(data.nodes);
            $('#nodes').unblock();
        });
    });

    $('a#mapping_results-btn').click(function(e) {
        $.post('/applications/' + current_application + '/deploy/map', function(data) {
            // Already an object
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                var $table = $('#mapping_results table tbody');
                $table.empty();

                _.each(data.mapping_results, function(result) {
                    if (result.instances.length > 0) {
                        _.each(result.instances, function(instance) {
                          if (instance.portNumber) {
                            if (instance.virtual) {
                              $table.append(_.template('<tr class=warning><td><%= instanceId %></td><td>(Virtual) <%= name %></td><td><%= nodeId %></td><td><%= portNumber %></td></tr>')(instance));
                            } else {
                              $table.append(_.template('<tr class=success><td><%= instanceId %></td><td><%= name %></td><td><%= nodeId %></td><td><%= portNumber %></td></tr>')(instance));
                            }
                          } else {
                            $table.append(_.template('<tr class=info><td><%= instanceId %></td><td><%= name %></td><td><%= nodeId %></td><td><%= portNumber %></td></tr>')(instance));
                          }
                        });
                    } else {
                        $table.append(_.template('<tr class=error><td><%= instanceId %></td><td><%= name %></td><td>Cannot find matching wuobjects</td><td></td></tr>')(result));
                    }
                });

                // print mapping status to #mapping-progress
                $('#mapping-progress').empty();
                for (var i=0; i<data.mapping_status.length; i++) {
                  $('#mapping-progress').append("<pre>[" + data.mapping_status[i].level + "]" + data.mapping_status[i].msg + "</pre>");
                }

                // disable deploy button if mapping is not successful
                if (!data.mapping_result) {
                  $('li a#deploy-btn').closest('li').hide();
                } else {
                  $('li a#deploy-btn').closest('li').show();
                }
            }
        });
    });

    // User clicking on deploy tab button (inner)
    $('a#deploy-btn').click(function(e) {
        e.preventDefault();
        $(this).tab('show');
		$('#deploy-progress').html('Start Deploy....');
        $.post('/applications/' + current_application + '/deploy', function(data) {
            // Already an object
            console.log('deploy signal set');
            if (data.status == 1) {
                alert(data.mesg);
            } else {
                // Print deploy status to #deploy-progress
                // Starts a new polling to deploy-progress
				setTimeout(function() {
	                application_polling(current_application, '#deploy-progress', 'deploy_status');
				},3000);
                //$('#deploy_results').dialog({modal: true, autoOpen: true, width: 600, height: 300}).dialog('open').bind('dialogclose', function(event, ui) {
                    //$('#deploy_results').dialog("close");
                //});
            }
        });
    });
});
