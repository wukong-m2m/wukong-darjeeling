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
            $('#log').append('<pre>' + data.log + '</pre>');
        });
    });

    $('#exclude').click(function() {
        console.log('exclude');
        $('#log').html('<h4>The basestation is ready to exclude devices.</h4>');
        $.post('/testrtt/exclude', function(data) {
            $('#log').append('<pre>' + data.log + '</pre>');
        });
    });

    $('#stop').click(function() {
        console.log('stop');
        $('#log').html('<h4>The basestation is stopped from adding/deleting devices.</h4>');
        $.post('/testrtt/stop', function(data) {
            $('#log').append('<pre>' + data.log + '</pre>');
        });
    });

    // starts polling
    poll('/testrtt/poll', 0, options);


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
