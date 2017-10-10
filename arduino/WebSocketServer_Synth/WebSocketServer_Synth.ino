/*
 * WebSocketServer_Synth.ino
 *
 *  
 *
 */

#include <WiFi.h>
#include <WebSocketsServer.h>
// INIT WEBSOCKET
WebSocketsServer webSocket = WebSocketsServer(81);
#define USE_SERIAL Serial
#define MESSAGE_INTERVAL 10
bool isConnected = false;
uint64_t messageTimestamp = 0;
float sensorValue;
int sensorPin = A2;

//HTTP web server on port 80
WiFiServer server(80);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    Serial.println("event happened "+num);
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
        break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        				isConnected = true;
        				// send message to client
        				webSocket.sendTXT(num, "Connected");
            }
        break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            // send message to client
             webSocket.sendTXT(num, "message here");
        break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
        break;
    }
}

void setup() {
    //CONNECTION ROUTINE
    USE_SERIAL.begin(115200);
    USE_SERIAL.setDebugOutput(true);
    // SLOWLY BOOTING
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    
    connectWifi();  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //websocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    //HTTP
    server.begin();
}

void loop() {
  webSocket.loop();
   if(isConnected) {
      uint64_t now = millis();
      sensorValue = analogRead(A2);
      //&& sensorValue>5.00
      if(now - messageTimestamp > MESSAGE_INTERVAL ) {
          messageTimestamp = now;
          webSocket.broadcastTXT("{\"luminosity\":\""+String(sensorValue)+"\"}");
      }
   }
   //HTTP SERVER
  WiFiClient client = server.available();    // listen for incoming clients
  if (client) {                              // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                 // make a String to hold incoming data from the client
    while (client.connected()) {             // loop while the client's connected
      if (client.available()) {              // if there's bytes to read from the client,
        char c = client.read();              // read a byte, then
        Serial.write(c);                     // print it out the serial monitor
        if (c == '\n') {                     // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // the content of the HTTP response follows the header:
            client.print("<html> <head><title>App</title><style>body{margin:0px}#synth-pad {background: #1E1E1E url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAYAAACNMs+9AAAAIUlEQVQYV2P8+vWrJAMRgBGkkJub+zkhtaMK8YYQ0cEDABxGJwNr5yClAAAAAElFTkSuQmCC);}</style></head><body data-IP=\"192.168.4.1\"><canvas id=\"synth-pad\"></canvas></body></html>");
            client.print("<script type=\"text/javascript\">'use strict';class SynthPad{constructor(){this.connection;this.IP=document.body.getAttribute('data-IP');this.myCanvas;this.myAudioContext;this.oscillator;this.gainNode;this.synthActive=!1;this.lowNote=10;this.highNote=800;this.myCanvas=document.getElementById('synth-pad');this.myCanvas.width=window.innerWidth;this.myCanvas.height=window.innerHeight;window.AudioContext=window.AudioContext||window.webkitAudioContext;this.myAudioContext=new window.AudioContext();this.init()};init(){window.WebSocket=window.WebSocket||window.MozWebSocket,window.WebSocket?this.initConnection():alert('Il faut utiliser un autre navigateur. Chrome par exemple.')};initConnection(){this.connection=new WebSocket('ws://'+this.IP+':81/');this.connection.onopen=this.onOpen.bind(this);this.connection.onerror=this.onError.bind(this);this.connection.onmessage=this.onMessage.bind(this)};onOpen(){this.setupEventListeners()};onError(error){console.log('!!',error)};onMessage(message){try{let o=JSON.parse(message.data);console.log(parseFloat(o.luminosity));if(parseInt(o.luminosity)>10&&!this.synthActive){this.playSound({'type':'mousemove','x':o.luminosity,'y':1})}else if(parseInt(o.luminosity)>6){this.updateFrequency({'type':'mousemove','x':o.luminosity,'y':1})}else{this.stopSound(null)}}catch(error){console.log('Error with json',error)}};setupEventListeners(){document.body.addEventListener('touchmove',function(event){event.preventDefault()},!1);this.myCanvas.addEventListener('mousedown',this.playSound.bind(this));this.myCanvas.addEventListener('touchstart',this.playSound.bind(this));this.myCanvas.addEventListener('mouseup',this.stopSound.bind(this));document.addEventListener('mouseleave',this.stopSound.bind(this));this.myCanvas.addEventListener('touchend',this.stopSound.bind(this))};playSound(event){console.log('playSound on mousedown');this.synthActive=!0;this.oscillator=this.myAudioContext.createOscillator();this.gainNode=this.myAudioContext.createGain();this.oscillator.type='triangle';this.gainNode.connect(this.myAudioContext.destination);this.oscillator.connect(this.gainNode);this.updateFrequency(event);this.oscillator.start(0);this.myCanvas.addEventListener('mousemove',this.updateFrequency.bind(this));this.myCanvas.addEventListener('touchmove',this.updateFrequency.bind(this));this.myCanvas.addEventListener('mouseout',this.stopSound.bind(this))};stopSound(event){this.oscillator.stop(0);this.synthActive=!1;this.myCanvas.removeEventListener('mousemove',this.updateFrequency.bind(this));this.myCanvas.removeEventListener('touchmove',this.updateFrequency.bind(this));this.myCanvas.removeEventListener('mouseout',this.stopSound.bind(this))};calculateNote(posX){let noteDifference=this.highNote-this.lowNote;let noteOffset=(noteDifference/this.myCanvas.offsetWidth)*(posX-this.myCanvas.offsetLeft);return this.lowNote+noteOffset};calculateVolume(posY){let volumeLevel=1-(((100/this.myCanvas.offsetHeight)*(posY-this.myCanvas.offsetTop))/100);return volumeLevel};calculateFrequency(x,y){let noteValue=this.calculateNote(x);let volumeValue=this.calculateVolume(y);this.oscillator.frequency.value=noteValue;this.gainNode.gain.value=volumeValue};updateFrequency(event){if(event.type=='mousedown'||event.type=='mousemove'){this.calculateFrequency(event.x,event.y)}else if(event.type=='touchstart'||event.type=='touchmove'){let touch=event.touches[0];this.calculateFrequency(touch.pageX,touch.pageY)}}};window.onload=function(){let synthPad=new SynthPad()}</script>");
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

boolean connectWifi(){
  boolean state = true;
  int i = 0;
  //CREATE A NETWORK
  WiFi.softAP("Matchbox_WIFI");
  return state;
}
