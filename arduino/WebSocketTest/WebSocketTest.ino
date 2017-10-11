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
            client.print("<html> <head><title>App</title></head><body data-IP=\"192.168.4.1\"></body></html>");
            client.print("<script type=\"text/javascript\">var SocketConnection=function(){this.connection;this.IP=document.body.getAttribute('data-IP');this.init()};SocketConnection.prototype={init:function(){window.WebSocket=window.WebSocket||window.MozWebSocket;window.WebSocket?this.initConnection():alert('Change browser please')},initConnection:function(){console.log('initConnection');this.connection=new WebSocket('ws://'+this.IP+':81/');this.connection.onopen=this.onOpen.bind(this);this.connection.onerror=this.onError.bind(this);this.connection.onmessage=this.onMessage.bind(this)},onOpen:function(){console.log('websocket connection ok')},onError:function(n){console.log('!!',n)},onMessage:function(n){try{var o=JSON.parse(n.data);document.body.innerHTML+=o.luminosity+'<br/>';console.log(o)}catch(e){return void console.log('Bad JSON format')}}};function launch(n){new SocketConnection()};window.addEventListener('DOMContentLoaded',launch);</script>");
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
