
connection = new WebSocket(wsUri, ['arduino']);
connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  handleRequest(e.data);
};

function handleRequest(request) {
  switch (request) {
    case "1":
      console.log('handleRequest: Autorecord prepare');
      console.log('spotify.pause()');
      setTimeout(() => { console.log('spotify.seek(0);'); }, 800);
      setTimeout(() => { console.log('spotify.setVolume(100);'); }, 1600);
      break;
    case "2":
      console.log('handleRequest function: Autorecord PLAY');
      console.log('spotify.play()');
      break;
    default:
      console.log('handleRequest function: Default');
  }
};


/* Test button */
document.getElementById("cplay").addEventListener("click", testFunc, false);
function testFunc() {
  console.log('Trigger from Web Client to ESP');
  connection.send('4');
};

document.getElementById("writeTocMd").addEventListener("click", function () {
  if ("" !== document.getElementById('album').value && "" !== document.getElementById('songs').value) {
    connection.send('0' + document.getElementById('album').value);
    connection.send('1' + document.getElementById('songs').value);
    connection.send('2' + document.getElementById('md').checked);
  }
});

document.getElementById("recordSpotify").addEventListener("click", function () {
  if ("spotify" !== document.getElementById('spoturi').value.split(':')[0]) {
    alert("Not a Spotify URI");
  } else {
    if ("album" === document.getElementById('spoturi').value.split(':')[1]) {
      connection.send('3' + document.getElementById('spoturi').value.split(':')[2]);
      console.log('Recording from Spotify!');
    } else {
      alert("Not an album URI");
    }
  }
});


document.getElementById("populateToc").addEventListener("click", function () {
  /* Pupulate TOC using Spotify and the URI above. Alternatively the currently playint track */
  document.getElementById('album').value = 'Bruce Springsteen - Born in the USA';
  document.getElementById('songs').value = 'First song\nSecond song\nLast song but not worst.\n';
});

document.getElementById("clearToc").addEventListener("click", function () {
  document.getElementById('album').value = "";
  document.getElementById('songs').value = "";
  console.log('TOC cleared');
});
