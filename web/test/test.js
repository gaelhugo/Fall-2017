'use strict';
class SocketConnection {
  constructor() {
    this.IP = document.body.getAttribute('data-IP');
    this.init();
  };

  init() {
    window.WebSocket = window.WebSocket || window.MozWebSocket;
    window.WebSocket ? this.initConnection() : alert('Change browser please');
  };

  initConnection() {
    console.log('initConnection');
    this.connection = new WebSocket('ws://' + this.IP + ':81/');
    this.connection.onopen = this.onOpen.bind(this);
    this.connection.onerror = this.onError.bind(this);
    this.connection.onmessage = this.onMessage.bind(this);
  };

  onOpen() {
    console.log('websocket connection ok');
  };

  onError(n) {
    console.log('!!', n);
  };

  onMessage(n) {
    try {
      let o = JSON.parse(n.data);
      document.body.innerHTML += o.luminosity + '<br/>';
      console.log(o);
    } catch (e) {
      console.log('Bad JSON format', e);
    }
  };
};

function launch(n) {
  new SocketConnection();
};

window.addEventListener('DOMContentLoaded', launch);
