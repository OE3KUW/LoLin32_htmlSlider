var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
    getCurrentValue();
}

function initWebSocket() {
    console.log('Trying to open a WebSockeet connection. . . ');
    websocket= new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    var text;
    var len;

    // console.log(event.data);

    if (event.data.charAt(0) == 'O')  // ON / OFF
    {
        console.log("event.data ON / OFF");
        document.getElementById('state').innerHTML = event.data;
    }    
    
/*    if (event.data.charAt(0) == 'B')  // BAT
    {
        len = event.data.length;
        // console.log(len);
        text = event.data.substring(3, len);  // von bis! 
        // console.log(text);
        document.getElementById('battery').innerHTML = text;
    }

    if (event.data.charAt(0) == 'W')
    {
        len = event.data.length;
        text = event.data.substring(3, len);
        var svgRectA = document.getElementById("meinRectA");
        var svgRectB = document.getElementById("meinRectB");
        var winkel = parseInt(text);
        svgRectA.setAttribute("transform","rotate(" + winkel + ", 200, 150 )");
        svgRectB.setAttribute("transform","rotate(" + winkel + ", 200, 150 )");
        console.log("winkel" + winkel);
    }
  */  
}

function initButton() {
    document.getElementById('bON').addEventListener('click', toggleON);
    document.getElementById('bOFF').addEventListener('click', toggleOFF);
}

function toggleON(event) {
    websocket.send('bON'); console.log("toggleON");
}

function toggleOFF(event) {
    websocket.send('bOFF'); console.log("toggleOFF");
}

function getCurrentValue()
{
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("pwmSlider").value = this.responseText;
            //document.getElementById("textSliderValue").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "/currentValue", true);
    xhr.send();
}
function updateSliderPWM(element) {
    var sliderValue = document.getElementById("pwmSlider").value;
    //document.getElementById("textSliderValue").innerHTML = sliderValue;
    console.log(sliderValue);
    websocket.send(sliderValue);
}
