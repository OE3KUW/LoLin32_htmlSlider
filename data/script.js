let gateway = `ws://${window.location.hostname}/ws`;
let websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
    getCurrentValue();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection. . . ');
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

    if (event.data.charAt(0) == 'O')  // ON / OFF
    {
        console.log("event.data ON / OFF");
        document.getElementById('state').innerHTML = event.data;
    }    
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
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("pwmSlider").value = this.responseText;
        }
    };
    xhr.open("GET", "/currentValue", true);
    console.log("xhr.send");
    xhr.send();
}
function updateSliderPWM(element) {
    let sliderValue = document.getElementById("pwmSlider").value;
    sliderValue = "sLa" + sliderValue;
    console.log(sliderValue);
    websocket.send(sliderValue);
}
