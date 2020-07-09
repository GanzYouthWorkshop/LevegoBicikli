/***********************************************************
Android externals
***********************************************************/
function ext_dataReceived(data)
{
  var percent = data / 1024 * 100;
  $("#current-power").html(data + ' W');
  $("#current-power").css('background-image', 'linear-gradient(0deg, var(--color-graph) ' + percent + '%, rgba(0, 0, 0, 0) ' + (percent + 20) + '%)');
  
  if(gameRunning)
  {
	  gameData.push(data);
  }
}

function ext_stateChanged(state)
{
  if(state == "OK")
  {
    $("#status").css('background-color', 'var(--color-ok)');
  }
  else
  {
    $("#status").css('background-color', 'var(--color-error)');
  }
}

/***********************************************************
Game
***********************************************************/
var gameData = [];

var gameRunning = false;
var globalTimer = 0;
var localTimer = 0;

function gameStart()
{
	$('#progressbar').css('width', '100%');
	$('#startstop').html('STOP');
	
	gameRunning = true;
	globalTimer = 0;
	localTimer = 0;
	gameData = [];

	setTimeout(gameStep, 100);
}

function gameStep()
{
	if(gameRunning)
	{
		globalTimer += 100;
		localTimer += 100;
		
		if(localTimer >= 1000)
		{
			$('#progressbar').css('width', ((30000 - globalTimer) / 30000 * 100) + '%');
			localTimer = 0;
		}
		
		if(globalTimer >= 30000)
		{
			gameStop();
			return;
		}
		else
		{
			setTimeout(gameStep, 100);
		}
		
		var avg = 0;
		var len = gameData.length;
		for(i = 0; i < len; i++)
		{
			avg += gameData[i];
		}
		avg /= len;
		
		$('#score').html(avg.toFixed(2) + " Wh")
	}
	else
	{
		gameStop();
	}
}

function gameStop()
{
	gameRunning = false;
	
	$('#progressbar').css('width', '100%');
	$('#startstop').html('START');
}

/***********************************************************
Debug
***********************************************************/
//setInterval(function(){ ext_dataReceived(Math.floor(Math.random() * 100)) }, 100);

/***********************************************************
UI
***********************************************************/
$('#startstop').click(function()
{
	gameRunning ? gameStop() : gameStart();
});