$(function() {

    // testrtt
    $('#stop').hide();
    $('#include, #exclude').click(function() {
        $('#include').hide();
        $('#exclude').hide();
        $('#stop').show();
    });

    $('#stop').click(function() {
        $('#include').show();
        $('#exclude').show();
        $('#stop').hide();
    });

    window.options = {repeat: true};

    $('#include').click(function() {
        console.log('include');
        $('#log').html('<h4>The basestation is ready to include devices.</h4>');
        $.post('/testrtt/include', function(data) {
            $('#log').html('<pre>' + data.logs.join("\n") + '</pre>');
        });
    });

    $('#exclude').click(function() {
        console.log('exclude');
        $('#log').html('<h4>The basestation is ready to exclude devices.</h4>');
        $.post('/testrtt/exclude', function(data) {
            $('#log').append('<pre>' + data.logs + '</pre>');
        });
    });

    $('#stop').click(function() {
        console.log('stop');
        $('#log').html('<h4>The basestation is stopped from adding/deleting devices.</h4>');
        $.post('/testrtt/stop', function(data) {
            $('#log').append('<pre>' + data.logs + '</pre>');
        });
    });

    // starts polling
    poll('/testrtt/poll', 0, options,function(data) {
		try {
				var s = data.logs.join("");
				if (s.indexOf("learn ready")!= -1) {
					s = "Ready to include/exclude device";
				} else if (s.indexOf("node found") != -1) {
					s = "Node is found";
				} else {
					s = data.logs.join("\n");
				}
				if (data.logs.join("") != "")
					$('#log').html('<pre>'+s+'</pre>');
		} catch(e) { }
	});


    // node discovery
    $('#myModal').hide();
    $('#dispObj').hide();

//    $('#nodes a').click(function(e) {
  //      e.preventDefault();
    //    $(this).tab('show');
    //});
    $('#location_editor_tab').click(function(e) {
        e.preventDefault();
        $(this).tab('show');
        $.post('/loc_tree', function(data) {
            $('#location_editor').empty();
            display_tree('#location_editor',data);
            $('#location_editor').append(data.node);
            document.body.dataset.installer_mode = "true";               
        });                    
    });
});
