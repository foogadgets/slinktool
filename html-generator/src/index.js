let SpotifyWebApi = require('spotify-web-api-node');
let ReconnectingWebSocket = require('reconnectingwebsocket');


let spotifyApi = new SpotifyWebApi({
  clientId: sessionStorage.clientId,
  clientSecret: sessionStorage.clientSecret
});

spotifyApi.setAccessToken(sessionStorage.accessToken);

document.getElementById('album').value = localStorage.slinktoolAlbum;
document.getElementById('songs').value = localStorage.slinktoolTracks;
document.getElementById('numberOfTracks').innerHTML = localStorage.slinktoolNumberOfTracks;
document.getElementById('playTime').innerHTML = localStorage.slinktoolPlayTime;

let funcToken = 0;

connection = new ReconnectingWebSocket(wsUri, ['arduino']);
connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  handleRequest(e.data);
};
connection.reconnectInterval = 2000;


function handleRequest(request) {
  let state = request.split(':')[0];
  let payload = request.split(':')[1];
  switch (state) {
    case 'p':
      console.log("spotify.play");
      spotifyApi.play({
        "context_uri": document.getElementById('spoturi').value,
        "offset": {
          "position": 0
        },
        "position_ms": 0
      }).then(function () {
        console.log('Spotify Play requested -------');
      }, function (err) {
        console.log('Something went wrong!', err);
      });
      break;
    case 't':
      sessionStorage.accessToken = payload;
      spotifyApi.setAccessToken(sessionStorage.accessToken);
      console.log('Got access token');
      populateToc();
      break;
    case 'm':
      console.log('TOC writing status: ');
      console.log(payload);
      switch (funcToken) {
        case 1:
          connection.send('3');
          funcToken = 0;
          break;
        case 2:
          connection.send('4');
          funcToken = 0;
          break;
        default:
      }
      break;
    case 'x':
      console.log('Done Recording to MD with reason ', payload);
      break;
    default:
      console.log('handleRequest function: Default');
  }
}


function writeTocToMd() {
  if (document.getElementById('album').value !== "" && document.getElementById('songs').value !== "") {
    funcToken = 2;
    sendMDToc();
  }
}
document.getElementById('writeTocMd').addEventListener('click', writeTocToMd, false);


function recordSpotify() {
  funcToken = 1;
  renewAccessToken();
}
document.getElementById('recordSpotify').addEventListener('click', recordSpotify, false);


function msToTime(s) {
  var ms = s % 1000;
  s = (s - ms) / 1000;
  var secs = s % 60;
  var mins = (s - secs) / 60;

  return mins + ':' + secs.toLocaleString(undefined, { minimumIntegerDigits: 2 });
}

function renewAccessToken() {
  connection.send('7');/* token renewal */
}
document.getElementById('populateToc').addEventListener('click', renewAccessToken, false);


function populateToc() {
  spotifyApi.getMyCurrentPlayingTrack().then(
    function (data) {
      localStorage.albumTimes = '';
      document.getElementById('songs').value = '';

      if ('playlist' === data.body.context.type) {
        document.getElementById('spoturi').value = data.body.context.uri;
        spotifyURI = document.getElementById('spoturi').value.split(':')[document.getElementById('spoturi').value.split(':').length - 1];
        spotifyApi.getPlaylist(spotifyURI, { limit: 100, offset: 0 }).then(
          function (data) {
            var playTime = 0;
            var noTracks = 0;
            var maxPlayTime = (((document.querySelector('input[name="mdLen"]:checked').value) * 60 + 59) * (document.querySelector('input[name="mdLP"]:checked').value) * 1000);
            document.getElementById('album').value = data.body.name;
            localStorage.slinktoolAlbum = document.getElementById('album').value;
            document.getElementById('imageUrl').src = data.body.images[0].url;
            noTracks = data.body.tracks.total;
            if (noTracks > 100) { noTracks = 100; }
            for (i = 0; i < noTracks; i++) { /* iterate all tracks */
              document.getElementById('songs').value += data.body.tracks.items[i].track.name;
              if ((document.querySelector('input[name="mdLP"]:checked').value) == 1) {
                document.getElementById('songs').value += ' - ' +
                  data.body.tracks.items[i].track.artists[0].name;
              }
              document.getElementById('songs').value += '\n';
              playTime += data.body.tracks.items[i].track.duration_ms;
              localStorage.albumTimes += data.body.tracks.items[i].track.duration_ms.toString() + ';';
              if (playTime > maxPlayTime) { noTracks = i + 1; break; }
            }
            document.getElementById('playTime').innerHTML = msToTime(playTime);
            document.getElementById('numberOfTracks').innerHTML = noTracks;
            storeToLocalStore();
          },
          function (err) {
            console.error(err);
          }
        );
      } else { /* It is not a playlist */
        document.getElementById('spoturi').value = data.body.item.album.uri;
        spotifyURI = document.getElementById('spoturi').value.split(':')[document.getElementById('spoturi').value.split(':').length - 1];
        spotifyApi.getAlbum(spotifyURI, { limit: 50, offset: 0 }).then(
          function (data) {
            var playTime = 0;
            var noTracks = 0;
            var maxPlayTime = (((document.querySelector('input[name="mdLen"]:checked').value) * 60 + 59) * (document.querySelector('input[name="mdLP"]:checked').value) * 1000);
            document.getElementById('album').value = data.body.artists[0].name + ' - ' + data.body.name;
            localStorage.slinktoolAlbum = document.getElementById('album').value;
            document.getElementById('imageUrl').src = data.body.images[0].url;
            noTracks = data.body.total_tracks;
            for (i = 0; i < noTracks; i++) { /* Iterate over all tracks */
              document.getElementById('songs').value += data.body.tracks.items[i].name + '\n';
              playTime += data.body.tracks.items[i].duration_ms;
              localStorage.albumTimes += data.body.tracks.items[i].duration_ms.toString() + ';';
              if (playTime > maxPlayTime) { noTracks = i + 1; break; }
            }
            document.getElementById('playTime').innerHTML = msToTime(playTime);
            document.getElementById('numberOfTracks').innerHTML = noTracks;
            storeToLocalStore();
          },
          function (err) {
            console.error(err);
          }
        );
      }
      sendMDToc();
    },
    function (err) {
      console.log('Something went wrong!', err);
    }
  );
}

function storeToLocalStore() {
  localStorage.slinktoolTracks = document.getElementById('songs').value;
  localStorage.slinktoolNumberOfTracks = document.getElementById('numberOfTracks').innerHTML;
  localStorage.slinktoolPlayTime = document.getElementById('playTime').innerHTML;
}

function sendMDToc() {
  connection.send('0' + document.getElementById('album').value);
  setTimeout(function () { connection.send('1' + document.getElementById('songs').value); }, 500);
  setTimeout(function () { connection.send('2' + localStorage.albumTimes); }, 2500);
  console.log('Sent Album, tracks and track duration.');
}

function clearToc() {
  document.getElementById('album').value = "";
  document.getElementById('songs').value = "";
  localStorage.slinktoolAlbum = "";
  localStorage.slinktoolTracks = "";
  document.getElementById('playTime').innerHTML = '';
  localStorage.slinktoolPlayTime = document.getElementById('playTime').innerHTML;
  document.getElementById('numberOfTracks').innerHTML = '';
  localStorage.slinktoolNumberOfTracks = document.getElementById('numberOfTracks').innerHTML;

  console.log('TOC cleared');
}
document.getElementById('clearToc').addEventListener('click', clearToc, false);

function interruptRec() {
  connection.send('8');
  console.log('Recording interrupted');
}
document.getElementById('stopRec').addEventListener('click', interruptRec, false);
