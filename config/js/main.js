
(function() {
  window.quotes = new Array();

  if ( getQueryParam ( "watch_platform" ) == "aplite" ) {
    $("#bgcolor").parent().hide();
    $("#quoteUP,#quoteDOWN").attr ("maxlength", 121);
  } else if ( getQueryParam ( "watch_platform" ) == "basalt" ) {
    $("#quoteUP,#quoteDOWN").attr ("maxlength", 141);
  }

  loadConfiguration();
  setEventHandlers();
})();

function loadConfiguration ( ) {
  if ( localStorage.config ) {
    $("#bgcolor")[0].value        = localStorage.bgcolor;
    $("#time24hours")[0].checked  = localStorage.time24hours === '1';
    $("#showcalendar")[0].checked = localStorage.showcalendar === '1';
    $("#showbattery")[0].checked  = localStorage.showbattery === '1';
    $("#changequote")[0].value    = localStorage.changequote;
    $("#changequotes")[0].value   = localStorage.changequotes;
    // window.quotes = JSON.parse( localStorage.quotes );
    window.quotes = JSON.parse ( getQueryParam ( "quotes" ) );
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
  localStorage.showbattery  = config.showbattery;
  localStorage.changequote  = config.changequote;
  localStorage.changequotes = config.changequotes;
  // localStorage.quotes       = JSON.stringify( window.quotes );
}

function refreshquotes( ) {
  console.log( 'Refreshing quotes ' + window.quotes );
  $('.item_quote').remove();

  if( window.quotes.length != 0 )
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
  $('#save_button').on('click', function(){
    var
      addquoteup  = $('#quoteUP').val(),
      addauthorup = $('#authorUP').val();
    // Validate for non empty strings
    if ( addquoteup.length != 0 && addauthorup.length != 0) {
      window.quotes.push( [ addquoteup, addauthorup ] );
    }

    var
      addquotedown  = $('#quoteDOWN').val(),
      addauthordown = $('#authorDOWN').val();
    // Validate for non empty strings
    if ( addquotedown.length != 0 && addauthordown.length != 0) {
      window.quotes.unshift( [ addquotedown, addauthordown ] );
    }

    var
      time24hours  = $("#time24hours")[0].checked  ? 1 : 0,
      showcalendar = $("#showcalendar")[0].checked ? 1 : 0,
      showbattery  = $("#showbattery")[0].checked  ? 1 : 0,
      changequote  = parseInt ( $("#changequote").val() ),
      changequotes = parseInt ( $("#changequotes").val() ),
      config = {
        'bgcolor'     : $("#bgcolor").val(),
        'time24hours' : time24hours,
        'showcalendar': showcalendar,
        'showbattery' : showbattery,
        'changequote' : changequote,
        'changequotes': changequotes,
        'quotes'      : window.quotes
      };

    saveConfiguration( config );

    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent ( JSON.stringify ( config ) );
  });

  $('#addquoteUP').on('click', function() {
    $('#quoteUP').focus();
    $('#addQuoteDialogUP').toggle();
    $('#addQuoteDialogDOWN').hide();
    location.href = '#addQuoteDialogAnchorUP';
  });

  $('#quoteUP').on( 'input', function() {
    $('#remainingUP').text( parseInt( $("#quoteUP").attr( "maxlength" ) ) - parseInt ( $('#quoteUP').val().length ) );
  });

  $('#quoteDOWN').on( 'input', function() {
    $('#remainingDOWN').text( parseInt( $("#quoteDOWN").attr( "maxlength" ) ) - parseInt ( $('#quoteDOWN').val().length ) );
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

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
