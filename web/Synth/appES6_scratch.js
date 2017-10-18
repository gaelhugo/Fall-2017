'use strict';

class App {
  constructor() {
    // WEB SOCKET
    this.connection;
    this.IP = document.body.getAttribute('data-IP');

    // Variables
    this.myCanvas;
    this.myAudioContext;
    this.oscillator;
    this.gainNode;
    this.synthActive = false;
    // Notes
    // C4
    this.lowNote = 10;
    // 261.63;
    // B4
    this.highNote = 800;
    // 493.88;

    // FOR VISUAL OUPUT
    this.myCanvas = document.getElementById('synth-pad');
    this.myCanvas.width = this.w = window.innerWidth;
    this.myCanvas.height = this.h = window.innerHeight;
    this.ctx = this.myCanvas.getContext('2d');


    // Create an audio context.
    window.AudioContext = window.AudioContext || window.webkitAudioContext;
    this.myAudioContext = new window.AudioContext();
    this.init();
  };

  init() {
    // check if websocket are available
    window.WebSocket = window.WebSocket || window.MozWebSocket,
    window.WebSocket ? this.initConnection() :
                       alert('Must use a modern browser.');
  };

  initConnection() {
    // setup websocket connection + listeners
    this.connection = new WebSocket('ws://' + this.IP + ':81/');
    this.connection.onopen = this.onOpen.bind(this);
    this.connection.onerror = this.onError.bind(this);
    this.connection.onmessage = this.onMessage.bind(this);
  };

  onOpen() {
    this.setupEventListeners();
    this.draw();
  };

  onError(error) {
    console.log('!!', error);
    // we set the listener anyway, so we can test the code
    this.setupEventListeners();
    this.draw();
  };

  onMessage(message) {
    // when a socket is received
    // we fake the interaction (sound frequency only)
    try {
      let o = JSON.parse(message.data);
      if (parseInt(o.luminosity) > 10 && !this.synthActive) {
        this.playSound({
          'type': 'mousemove',
          'x': o.luminosity,
          'y': window.innerHeight / 2,
        });
      } else if (parseInt(o.luminosity) > 10) {
        this.updateFrequency({
          'type': 'mousemove',
          'x': o.luminosity,
          'y': window.innerHeight / 2,
        });
      } else {
        this.stopSound(null);
      };
    } catch (error) {
      console.log('Error with json', error);
    };
  };

  // Event Listeners
  setupEventListeners() {
    // Disables scrolling on touch devices.
    document.body.addEventListener('touchmove', function(event) {
      event.preventDefault();
    }, false);

    // to interact with the finger
    this.myCanvas.addEventListener('mousedown', this.playSound.bind(this));
    this.myCanvas.addEventListener('touchstart', this.playSound.bind(this));
    this.myCanvas.addEventListener('mouseup', this.stopSound.bind(this));
    document.addEventListener('mouseleave', this.stopSound.bind(this));
    this.myCanvas.addEventListener('touchend', this.stopSound.bind(this));
  };

  // Play a note.
  playSound(event) {
    console.log('playSound on mousedown');
    this.synthActive = true;
    this.oscillator = this.myAudioContext.createOscillator();
    this.gainNode = this.myAudioContext.createGain();
    // sine, square, sawtooth, triangle, custom
    this.oscillator.type = 'sine';
    this.gainNode.connect(this.myAudioContext.destination);
    this.oscillator.connect(this.gainNode);
    this.updateFrequency(event);
    this.oscillator.start(0);
    this.myCanvas.addEventListener(
        'mousemove', this.updateFrequency.bind(this));
    this.myCanvas.addEventListener(
        'touchmove', this.updateFrequency.bind(this));
    this.myCanvas.addEventListener('mouseout', this.stopSound.bind(this));
  };

  // Stop the audio.
  stopSound(event) {
    if (this.synthActive) {
      this.oscillator.stop(0);
      this.synthActive = false;
    };
    this.myCanvas.removeEventListener(
        'mousemove', this.updateFrequency.bind(this));
    this.myCanvas.removeEventListener(
        'touchmove', this.updateFrequency.bind(this));
    this.myCanvas.removeEventListener('mouseout', this.stopSound.bind(this));
    // mouse.x = 0;
    // mouse.y = 0;
  };

  // Update the note frequency.
  updateFrequency(event) {
    if (event.type == 'mousedown' || event.type == 'mousemove') {
      this.calculateFrequency(event.x, event.y);
      // mouse.x = event.x;
      // mouse.y = event.y;
    } else if (event.type == 'touchstart' || event.type == 'touchmove') {
      let touch = event.touches[0];
      this.calculateFrequency(touch.pageX, touch.pageY);
      // mouse.x = touch.pageX;
      // mouse.y = touch.pageY;
    }
  };

  // Fetch the new frequency and volume.
  calculateFrequency(x, y) {
    let noteValue = this.calculateNote(x);
    let volumeValue = this.calculateVolume(y);
    this.oscillator.frequency.value = noteValue;
    this.gainNode.gain.value = volumeValue;
  };

  // Calculate the note frequency.
  calculateNote(posX) {
    let noteDifference = this.highNote - this.lowNote;
    let noteOffset = (noteDifference / this.myCanvas.offsetWidth) *
        (posX - this.myCanvas.offsetLeft);
    return this.lowNote + noteOffset;
  };

  // Calculate the volume.
  calculateVolume(posY) {
    let volumeLevel = 1 -
        (((100 / this.myCanvas.offsetHeight) *
          (posY - this.myCanvas.offsetTop)) /
         100);
    return volumeLevel;
  };

  draw() {};
};

window.onload = function() {
  new App();
};
