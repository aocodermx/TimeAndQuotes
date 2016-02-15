(function() {
  loadConfiguration();
  setEventHandlers();
})();

function loadConfiguration ( ) {
  if ( localStorage.config ) {
    $("#bgcolor")[0].value        = localStorage.bgcolor;
    $("#time24hours")[0].checked  = localStorage.time24hours === 'true';
    $("#showcalendar")[0].checked = localStorage.showcalendar === 'true';
    $("#changequote")[0].value    = localStorage.changequote;
    quotes = JSON.parse( localStorage.quotes );
    refreshquotes();
  } else {
    console.log( "No configuration saved." );
  }
}


function saveConfiguration ( config ) {
  localStorage.config       = true;
  localStorage.bgcolor      = config.bgcolor;
  localStorage.time24hours  = config.time24hours;
  localStorage.showcalendar = config.showcalendar;
  localStorage.changequote  = config.changequote;
  localStorage.quotes       = JSON.stringify( quotes );
}

function refreshquotes() {
  console.log( 'Refreshing quotes ' + quotes );
  $('.item_quote').remove();

  /*
  if( quotes.length != 0 )
    $('#addquoteDOWN').show();
  else
    $('#addquoteDOWN').hide();
  */

  for ( var i=0; i<quotes.length; i++ ) {
    $('#addQuoteDialog').after( '<label class="item item_quote"><blockquote><p>' +
          quotes[i][0] +
          '</p><footer>' +
          quotes[i][1] +
          '</footer></blockquote><div class="delete-item" id="deletequote'+i+'">' +
            '<input type="hidden" id="index" value="' + i +'">' +
          '</div></label>');

    $( '#deletequote' + i ).on('click', function() {
      var index = $(this).children('#index').val();
      console.log('Delete item called on ' + index );
      quotes.splice( index, 1 );
      refreshquotes();
    });
  }

  $('#addQuoteDialog').hide();
}


function setEventHandlers ( ) {
  var $save_button = $('#save_button');

  $save_button.on('click', function(){
    var config = {
      'bgcolor'     : $("#bgcolor").val(),
      'time24hours' : $("#time24hours")[0].checked,
      'showcalendar': $("#showcalendar")[0].checked,
      'changequote' : $("#changequote").val(),
      'quotes'      : quotes
    };

    saveConfiguration( config );

    location.href = 'pebblejs://close#' +
      encodeURIComponent ( JSON.stringify ( config ) );
  });

  $('#addquoteUP').on('click', function() {
    $('#addQuoteDialog').toggle();
    location.href = '#addQuoteDialogAnchor';
  });

  $('#addquoteDOWN').on('click', function() {
    $('#addQuoteDialog').toggle();
  });

  $('#addquoteconfirm').on('click', function() {
    var addquote  = $('#quote').val();
    var addauthor = $('#author').val();
    // Validate for non empty strings
    if ( addquote.length != 0 && addauthor.length != 0) {
      quotes.push( [ addquote, addauthor ] );
      $('#quote').val("");
      $('#author').val("");
      refreshquotes();
    } else {
      console.log("Add a valid author and quote.");
    }
  });

  $('#addquotecancel').on('click', function() {
    $('#quote').val("");
    $('#author').val("");
    $('#addQuoteDialog').hide();
  });
}
