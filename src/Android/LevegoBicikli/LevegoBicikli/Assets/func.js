function ext_dataReceived(data)
{
  var percent = data / 1024 * 100;
  $("#current-power").html(data + ' kW');
  $("#current-power").css('background-image', 'linear-gradient(0deg, blue ' + percent + '%, rgba(0, 0, 0, 0) ' + (percent + 20) + '%)');
}

function ext_stateChanged(state)
{
  if(state == "OK")
  {
    $("#status").css('background-color', 'green');
  }
  else
  {
    $("#status").css('background-color', 'red');
  }
}