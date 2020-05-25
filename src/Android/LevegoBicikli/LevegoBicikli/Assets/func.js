function ext_dataReceived(data)
{
  $("#current-power").html(data + ' kW');
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