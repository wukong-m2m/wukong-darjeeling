$(function() {

    window.options = {repeat: true};
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
            $('#landmark_editor').empty();
            display_tree('#location_editor',data);
            $('#location_editor').append(data.node);
            document.body.dataset.installer_mode = "true";               
            $('#dispTreeNodeInfo .title').text('Location Information')            
        });                    
    });
    
    $('#landmark_editor_tab').click(function(e) {
        e.preventDefault();
        $(this).tab('show');
        $('#location_editor_tab').parent().removeClass('active');
        $('#nodes_tab').parent().removeClass('active');
        $('#landmark_editor_tab').parent().addClass('active');
        window.options.repeat = false;
        $.post('/loc_tree', function(data) {
                $('#location_editor').empty();
                display_tree("#landmark_editor",data);
                $('#landmark_editor').append(data.node);
                document.body.dataset.installer_mode = "false";
                $('#dispTreeNodeInfo .title').text( 'Landmark Information')
        });
    });  
    
});
