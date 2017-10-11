var SocketConnection = function() {
  this.connection;
  this.IP = document.body.getAttribute('data-IP');
  this.init();
};
SocketConnection.prototype = {
  init: function() {
    window.WebSocket = window.WebSocket || window.MozWebSocket;
    window.WebSocket ? this.initConnection() : alert('Change browser please');
  },
  initConnection: function() {
    console.log('initConnection');
    this.connection = new WebSocket('ws://' + this.IP + ':81/');
    this.connection.onopen = this.onOpen.bind(this);
    this.connection.onerror = this.onError.bind(this);
    this.connection.onmessage = this.onMessage.bind(this);
  },
  onOpen: function() {
    console.log('websocket connection ok');
  },
  onError: function(n) {
    console.log('!!', n);
  },
  onMessage: function(n) {
    try {
      var o = JSON.parse(n.data);
      document.body.innerHTML += o.luminosity + '<br/>';
      console.log(o);
    } catch (e) {
      return void console.log('Bad JSON format');
    }
  }
};

function launch(n) {
  new SocketConnection();
};
window.addEventListener('DOMContentLoaded', launch);
