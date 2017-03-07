$(function() {
  $('#form1').submit(function() {
    console.log('submitting...');
    $(this).ajaxSubmit({
      beforeSubmit: function() {
        $('#status ul').append($('<li>uploading...</li>'));
      },
      success: function(data) {
        console.log(data);
        if (data.status == 0) {
          queryStatus(data.id);
        } else if (data.status == 1) {
          alert(data.mesg);
        }
      }
    });
    return false;
  });

  queryStatus = function(current_id) {
    $.ajax({
      type: 'POST',
      url: '/status',
      data: {id: current_id},
      async: true,
      cache: false,
      success: function(data) {
        console.log(data);
        if (data.status == 0) {
          if (data.current_status == 0) {
            setTimeout(function() {queryStatus(current_id)}, 10);
          } else if (data.current_status == 1) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('.progress .bar').css('width', '25%');
            setTimeout(function() {queryStatus(current_id)}, 10);
          } else if (data.current_status == 2) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            $('.progress .bar').css('width', '50%');
            setTimeout(function() {queryStatus(current_id)}, 10);
          } else if (data.current_status == 3) {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            $('#status ul').append($('<li>compiling...</li>'));
            $('.progress .bar').css('width', '75%');
            setTimeout(function() {queryStatus(current_id)}, 10);
          } else {
            $('#status ul').empty();
            $('#status ul').append($('<li>waiting...</li>'));
            $('#status ul').append($('<li>uploading...</li>'));
            $('#status ul').append($('<li>extracting...</li>'));
            $('#status ul').append($('<li>converting...</li>'));
            $('#status ul').append($('<li>compiling...</li>'));
            $('#status ul').append($('<li>done...</li>'));
            $('.progress .bar').css('width', '100%');
          }
        } else {
          setTimeout(function() {queryStatus(current_id)}, 10);
        }
      },
      error: function(request, status, error) {
        console.log('error thrown: ' + error);
        setTimeout(function() {queryStatus(current_id)}, 10);
      }
    });
  }

});
