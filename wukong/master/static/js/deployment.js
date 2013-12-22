

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
        $.post('/nodes/refresh/0', function(data) {
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
