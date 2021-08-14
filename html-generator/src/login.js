connection = new WebSocket(wsUri, ['arduino']);

connection.onopen = function () {
  connection.send('Connect ' + new Date());
};

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) {
  var state = e.data.split(':')[0];
  var payload = e.data.split(':')[1];
  switch(state) {
    case 'r':
      sessionStorage.refreshToken = payload;
     break;
    case 't':
      sessionStorage.accessToken = payload;
     break;
    default:
      console.log('handleRequest function: Default');
  }
};

document.getElementById('clid').value = localStorage.clientID;
document.getElementById('clse').value = localStorage.clientSecret;

function registerCredentials()
{
  localStorage.clientID = document.getElementById('clid').value;
  localStorage.clientSecret = document.getElementById('clse').value;

  connection.send('5' + document.getElementById('clid').value);
  connection.send('6' + document.getElementById('clse').value);

  const myUrl = new URL("https://accounts.spotify.com/authorize");
  myUrl.searchParams.append("response_type", "code");
  myUrl.searchParams.append("client_id", document.getElementById('clid').value);
  myUrl.searchParams.append("redirect_uri", "http://slinktool.local/callback");
  myUrl.searchParams.append("scope", "user-read-playback-state user-modify-playback-state");

  document.getElementById('redirectUrl').innerHTML = '<a href="' + myUrl + '">Spotify Login</a>';
};
document.getElementById("loginButton").addEventListener("click", registerCredentials, false);
