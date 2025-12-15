
var url = "ws://192.168.4.1:1337/";

function init() {  
	wsConnect(url);
	CubeBegin() ;	
}
	
function wsConnect(url) {  	
	websocket = new WebSocket(url);  	
	websocket.onopen = function(evt) { onOpen(evt) };
	websocket.onclose = function(evt) { onClose(evt) };
	websocket.onmessage = function(evt) { onMessage(evt) };
	websocket.onerror = function(evt) { onError(evt) };
}

function doSend(message) {
	console.log("Sending: " + message);
	websocket.send(message);
}

function onOpen(evt) {
    console.log("Connected");
}

function onClose(evt) {
    console.log('Connection closed');
}


function onError(evt) {
    console.log("ERROR: " + evt.data);
}

function onMessage(evt){
	var data = JSON.parse(evt.data);
	console.log(data);
	for (var i = 0; i < 16; i++) {
		document.getElementById("ang").innerHTML = ((360-data.map[i])% 360);
		document.getElementById("dist").innerHTML  = (data.distances[i])/10;
	}
	
}

function updatespeed(val) {
	document.getElementById('speedstep').value=val; 
	
}




window.addEventListener("load", init, false);




