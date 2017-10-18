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
        				//webSocket.sendTXT(num, "Connected");
                //webSocket.sendTXT(num,"{\"luminosity\":\"0\"}");
                 webSocket.broadcastTXT("{\"luminosity\":\""+String(sensorValue)+"\"}");
            }
        break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            // send message to client
             //webSocket.sendTXT(num, "message here");
             //webSocket.sendTXT(num,"{\"luminosity\":\"0\"}");
              webSocket.broadcastTXT("{\"luminosity\":\""+String(sensorValue)+"\"}");
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
            client.print("<script type=\"text/javascript\">'use strict';class SynthPad{constructor(){this.connection;this.IP=document.body.getAttribute('data-IP');this.myCanvas;this.myAudioContext;this.oscillator;this.gainNode;this.synthActive=!1;this.lowNote=10;this.highNote=800;this.myCanvas=document.getElementById('synth-pad');this.myCanvas.width=this.w=window.innerWidth;this.myCanvas.height=this.h=window.innerHeight;this.ctx=this.myCanvas.getContext('2d');this.ctx.font='150px sans-serif';this.txt='AMT ROCKS';this.ctx.fillStyle='white';this.ctx.fillText(this.txt,window.innerWidth/2-this.ctx.measureText(this.txt).width/2,window.innerHeight/2+50);let data32=new Uint32Array(this.ctx.getImageData(0,0,this.w,this.h).data.buffer);this.ctx.clearRect(0,0,window.innerWidth,window.innerHeight);this.vehicles=[];for(let i=0;i<data32.length;i+=64){if(data32[i]&0xff000000){this.vehicles.push(new Vehicle(i%this.w,i/this.w,this.ctx))}};window.AudioContext=window.AudioContext||window.webkitAudioContext;this.myAudioContext=new window.AudioContext();this.init()};init(){window.WebSocket=window.WebSocket||window.MozWebSocket,window.WebSocket?this.initConnection():alert('Must use a modern browser.')};initConnection(){this.connection=new WebSocket('ws://'+this.IP+':81/');this.connection.onopen=this.onOpen.bind(this);this.connection.onerror=this.onError.bind(this);this.connection.onmessage=this.onMessage.bind(this)};onOpen(){this.setupEventListeners();this.draw()};onError(error){console.log('!!',error);this.setupEventListeners();this.draw()};onMessage(message){try{let o=JSON.parse(message.data);if(parseInt(o.luminosity)>10&&!this.synthActive){this.playSound({'type':'mousemove','x':o.luminosity,'y':window.innerHeight/2,})}else if(parseInt(o.luminosity)>6){this.updateFrequency({'type':'mousemove','x':o.luminosity,'y':window.innerHeight/2,})}else{this.stopSound(null)}}catch(error){console.log('Error with json',error)}};setupEventListeners(){document.body.addEventListener('touchmove',function(event){event.preventDefault()},!1);this.myCanvas.addEventListener('mousedown',this.playSound.bind(this));this.myCanvas.addEventListener('touchstart',this.playSound.bind(this));this.myCanvas.addEventListener('mouseup',this.stopSound.bind(this));document.addEventListener('mouseleave',this.stopSound.bind(this));this.myCanvas.addEventListener('touchend',this.stopSound.bind(this))};playSound(event){console.log('playSound on mousedown');this.synthActive=!0;this.oscillator=this.myAudioContext.createOscillator();this.gainNode=this.myAudioContext.createGain();this.oscillator.type='square';this.gainNode.connect(this.myAudioContext.destination);this.oscillator.connect(this.gainNode);this.updateFrequency(event);this.oscillator.start(0);this.myCanvas.addEventListener('mousemove',this.updateFrequency.bind(this));this.myCanvas.addEventListener('touchmove',this.updateFrequency.bind(this));this.myCanvas.addEventListener('mouseout',this.stopSound.bind(this))};stopSound(event){if(this.synthActive){this.oscillator.stop(0);this.synthActive=!1};this.myCanvas.removeEventListener('mousemove',this.updateFrequency.bind(this));this.myCanvas.removeEventListener('touchmove',this.updateFrequency.bind(this));this.myCanvas.removeEventListener('mouseout',this.stopSound.bind(this));mouse.x=0;mouse.y=0};calculateNote(posX){let noteDifference=this.highNote-this.lowNote;let noteOffset=(noteDifference/this.myCanvas.offsetWidth)*(posX-this.myCanvas.offsetLeft);return this.lowNote+noteOffset};calculateVolume(posY){let volumeLevel=1-(((100/this.myCanvas.offsetHeight)*(posY-this.myCanvas.offsetTop))/100);return volumeLevel};calculateFrequency(x,y){let noteValue=this.calculateNote(x);let volumeValue=this.calculateVolume(y);this.oscillator.frequency.value=noteValue;this.gainNode.gain.value=volumeValue};updateFrequency(event){if(event.type=='mousedown'||event.type=='mousemove'){this.calculateFrequency(event.x,event.y);mouse.x=event.x;mouse.y=event.y}else if(event.type=='touchstart'||event.type=='touchmove'){let touch=event.touches[0];this.calculateFrequency(touch.pageX,touch.pageY);mouse.x=touch.pageX;mouse.y=touch.pageY}};draw(){this.ctx.clearRect(0,0,window.innerWidth,window.innerHeight);this.ctx.fillStyle='red';for(let i=0;i<this.vehicles.length;i++){this.vehicles[i].behaviors();this.vehicles[i].update();this.vehicles[i].show()};requestAnimationFrame(this.draw.bind(this))}};class Vehicle{constructor(x,y,ctx){this.ctx=ctx;this.pos={'x':Math.random()*window.innerWidth,'y':Math.random()*window.innerHeight,};this.vel={'x':1-Math.random()*2,'y':1-Math.random()*2};this.acc={'x':0,'y':0};this.target={'x':x,'y':y};this.desired={'x':0,'y':0};this.steering={'x':0,'y':0};this.r=2;this.maxspeed=10;this.maxforce=0.4;this.minReactivDistance=100};behaviors(){let _seek=this.seek(this.target);let _flee=this.flee(mouse);_flee.x*=40;_flee.y*=40;this.applyForce(_flee);this.applyForce(_seek)};applyForce(f){this.acc.x+=Number(f.x);this.acc.y+=Number(f.y)};seek(target){this.desired.x=Number(target.x-this.pos.x);this.desired.y=Number(target.y-this.pos.y);let mag=this.dist(this.target,this.pos);let d=mag;let speed=this.maxspeed;if(d<50){speed=d.map(0,50,0,this.maxspeed)};let uniform=speed/mag;this.desired.x*=uniform;this.desired.y*=uniform;this.steering.x=Number(this.desired.x-this.vel.x);this.steering.y=Number(this.desired.y-this.vel.y);this.steering.x=Math.min(Math.max(parseInt(this.steering.x),-this.maxforce),this.maxforce);this.steering.y=Math.min(Math.max(parseInt(this.steering.y),-this.maxforce),this.maxforce);return this.steering};flee(target){this.desired.x=Number(target.x-this.pos.x);this.desired.y=Number(target.y-this.pos.y);let mag=this.dist(target,this.pos);let d=mag;if(d<this.minReactivDistance){let uniform=this.maxspeed/mag;this.desired.x*=uniform*-1;this.desired.y*=uniform*-1;this.steering.x=Number(this.desired.x-this.vel.x);this.steering.y=Number(this.desired.y-this.vel.y);this.steering.x=Math.min(Math.max(parseInt(this.steering.x),-0.06),0.06);this.steering.y=Math.min(Math.max(parseInt(this.steering.y),-0.06),0.06);return this.steering}else{return{'x':0,'y':0}}};dist(target,pos){return Math.sqrt(Math.pow(target.x-pos.x,2)+Math.pow(target.y-pos.y,2))};update(){this.pos.x+=this.vel.x;this.pos.y+=this.vel.y;this.vel.x+=this.acc.x;this.vel.y+=this.acc.y;this.acc.x=0;this.acc.y=0};show(){this.ctx.beginPath();this.ctx.arc(this.pos.x,this.pos.y,this.r,0,Math.PI*2,!1);this.ctx.fill();this.ctx.closePath()}};let mouse={'x':0,'y':0};Number.prototype.map=function(in_min,in_max,out_min,out_max){return(this-in_min)*(out_max-out_min)/(in_max-in_min)+out_min};window.onload=function(){let synthPad=new SynthPad()}</script>");
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
