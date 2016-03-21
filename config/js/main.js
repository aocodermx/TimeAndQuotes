
(function() {
  window.quotes = new Array();
  loadConfiguration();
  setEventHandlers();
})();

function loadConfiguration ( ) {
  if ( localStorage.config ) {
    $("#bgcolor")[0].value        = localStorage.bgcolor;
    $("#time24hours")[0].checked  = localStorage.time24hours === '1';
    $("#showcalendar")[0].checked = localStorage.showcalendar === '1';
    $("#changequote")[0].value    = localStorage.changequote;
    $("#changequotes")[0].value   = localStorage.changequotes;
    window.quotes = JSON.parse( localStorage.quotes );
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
  localStorage.changequotes = config.changequotes;
  localStorage.quotes       = JSON.stringify( window.quotes );
}

function refreshquotes( ) {
  console.log( 'Refreshing quotes ' + window.quotes );
  $('.item_quote').remove();

  if( quotes.length != 0 )
    $('#addquoteDOWN').css('display', 'block');
  else
    $('#addquoteDOWN').hide();

  for ( var i=0; i<window.quotes.length; i++ ) {
    $('#addQuoteDialogUP').after( '<label class="item item_quote"><blockquote><p>' +
          window.quotes[i][0] +
          '</p><footer><strong>' +
          window.quotes[i][1] +
          '</strong></footer></blockquote><div class="delete-item" id="deletequote'+i+'">' +
            '<input type="hidden" id="index" value="' + i +'">' +
          '</div></label>');

    $( '#deletequote' + i ).on('click', function() {
      var index = $(this).children('#index').val();
      console.log('Delete item called on ' + index );
      window.quotes.splice( index, 1 );
      refreshquotes();
    });
  }

  $('#addQuoteDialogUP').hide();
  $('#addQuoteDialogDOWN').hide();
}


function setEventHandlers ( ) {
  var $save_button = $('#save_button');

  $save_button.on('click', function(){
    var
      time24hours  = $("#time24hours")[0].checked ? 1 : 0,
      showcalendar = $("#showcalendar")[0].checked ? 1 : 0,
      changequote  = parseInt ( $("#changequote").val() ),
      changequotes = parseInt ( $("#changequotes").val() ),
      config = {
        'bgcolor'     : $("#bgcolor").val(),
        'time24hours' : time24hours,
        'showcalendar': showcalendar,
        'changequote' : changequote,
        'changequotes': changequotes,
        'quotes'      : window.quotes
      };

    saveConfiguration( config );

    location.href = 'pebblejs://close#' +
      encodeURIComponent ( JSON.stringify ( config ) );
  });

  $('#addquoteUP').on('click', function() {
    $('#quoteUP').focus();
    $('#addQuoteDialogUP').toggle();
    $('#addQuoteDialogDOWN').hide();
    location.href = '#addQuoteDialogAnchorUP';
  });

  $('#addquoteDOWN').on('click', function() {
    $('#quoteDOWN').focus();
    $('#addQuoteDialogUP').hide();
    $('#addQuoteDialogDOWN').toggle();
    location.href = '#addQuoteDialogAnchorDOWN';
  });

  $('#addquoteconfirmUP').on('click', function() {
    var
      addquote  = $('#quoteUP').val(),
      addauthor = $('#authorUP').val();
    // Validate for non empty strings
    if ( addquote.length != 0 && addauthor.length != 0) {
      window.quotes.push( [ addquote, addauthor ] );
      $('#quoteUP').val("");
      $('#authorUP').val("");
      refreshquotes();
    } else {
      console.log("Add a valid author and quote.");
    }
  });

  $('#addquoteconfirmDOWN').on('click', function() {
    var
      addquote  = $('#quoteDOWN').val(),
      addauthor = $('#authorDOWN').val();
    // Validate for non empty strings
    if ( addquote.length != 0 && addauthor.length != 0) {
      window.quotes.unshift( [ addquote, addauthor ] );
      $('#quoteDOWN').val("");
      $('#authorDOWN').val("");
      refreshquotes();
    } else {
      console.log("Add a valid author and quote.");
    }
  });

  $('#addquotecancelUP').on('click', function() {
    $('#quoteUP').val("");
    $('#authorUP').val("");
    $('#addQuoteDialogUP').hide();
  });

  $('#addquotecancelDOWN').on('click', function() {
    $('#quoteDOWN').val("");
    $('#authorDOWN').val("");
    $('#addQuoteDialogDOWN').hide();
  });
}
