'use strict';
class SynthPad {
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
    // text
    this.ctx.font = '150px sans-serif';
    this.txt = 'AMT ROCKS';
    this.ctx.fillStyle = 'white';
    this.ctx.fillText(
        this.txt,
        window.innerWidth / 2 - this.ctx.measureText(this.txt).width / 2,
        window.innerHeight / 2 + 50);
    // get a Uint32 representation of the bitmap:
    let data32 = new Uint32Array(
        this.ctx.getImageData(0, 0, this.w, this.h).data.buffer);
    this.ctx.clearRect(0, 0, window.innerWidth, window.innerHeight);
    // wide grid
    // this.points = [];
    this.vehicles = [];
    for (let i = 0; i < data32.length; i += 64) {
      if (data32[i] & 0xff000000) {
        // this.points.push({
        //   x: (i % this.w),
        //   y: ((i / this.w)),
        // });
        this.vehicles.push(new Vehicle(i % this.w, i / this.w, this.ctx));
      }
    };

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
      } else if (parseInt(o.luminosity) > 6) {
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
    // sine, square, sawtooth, triangle,custom
    this.oscillator.type = 'square';
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
    mouse.x = 0;
    mouse.y = 0;
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

  // Fetch the new frequency and volume.
  calculateFrequency(x, y) {
    let noteValue = this.calculateNote(x);
    let volumeValue = this.calculateVolume(y);
    this.oscillator.frequency.value = noteValue;
    this.gainNode.gain.value = volumeValue;
  };

  // Update the note frequency.
  updateFrequency(event) {
    if (event.type == 'mousedown' || event.type == 'mousemove') {
      this.calculateFrequency(event.x, event.y);
      mouse.x = event.x;
      mouse.y = event.y;
    } else if (event.type == 'touchstart' || event.type == 'touchmove') {
      let touch = event.touches[0];
      this.calculateFrequency(touch.pageX, touch.pageY);
      mouse.x = touch.pageX;
      mouse.y = touch.pageY;
    }
  };

  draw() {
    this.ctx.clearRect(0, 0, window.innerWidth, window.innerHeight);
    this.ctx.fillStyle = 'red';
    for (let i = 0; i < this.vehicles.length; i++) {
      this.vehicles[i].behaviors();
      this.vehicles[i].update();
      this.vehicles[i].show();
    };
    requestAnimationFrame(this.draw.bind(this));
  };
};

class Vehicle {
  constructor(x, y, ctx) {
    this.ctx = ctx;
    this.pos = {
      'x': Math.random() * window.innerWidth,
      'y': Math.random() * window.innerHeight,
    };
    this.vel = {'x': 1 - Math.random() * 2, 'y': 1 - Math.random() * 2};
    this.acc = {'x': 0, 'y': 0};
    this.target = {'x': x, 'y': y};
    this.desired = {'x': 0, 'y': 0};
    this.steering = {'x': 0, 'y': 0};
    this.r = 2;
    this.maxspeed = 10;
    this.maxforce = 0.4;
    this.minReactivDistance = 100;
  };

  behaviors() {
    let _seek = this.seek(this.target);
    let _flee = this.flee(mouse);

    _flee.x *= 40;
    _flee.y *= 40;

    this.applyForce(_flee);
    this.applyForce(_seek);
  };

  applyForce(f) {
    this.acc.x += Number(f.x);
    this.acc.y += Number(f.y);
  };
  seek(target) {
    this.desired.x = Number(target.x - this.pos.x);
    this.desired.y = Number(target.y - this.pos.y);
    let mag = this.dist(this.target, this.pos);
    let d = mag;
    let speed = this.maxspeed;
    if (d < 50) {
      speed = d.map(0, 50, 0, this.maxspeed);
    };
    let uniform = speed / mag;
    this.desired.x *= uniform;
    this.desired.y *= uniform;
    // this.steering behaviour
    this.steering.x = Number(this.desired.x - this.vel.x);
    this.steering.y = Number(this.desired.y - this.vel.y);
    // limit to max speed
    this.steering.x = Math.min(
        Math.max(parseInt(this.steering.x), -this.maxforce), this.maxforce);
    this.steering.y = Math.min(
        Math.max(parseInt(this.steering.y), -this.maxforce), this.maxforce);
    return this.steering;
  };
  flee(target) {
    this.desired.x = Number(target.x - this.pos.x);
    this.desired.y = Number(target.y - this.pos.y);
    let mag = this.dist(target, this.pos);
    let d = mag;
    if (d < this.minReactivDistance) {
      let uniform = this.maxspeed / mag;
      // invert
      this.desired.x *= uniform * -1;
      this.desired.y *= uniform * -1;
      // this.steering behaviour
      this.steering.x = Number(this.desired.x - this.vel.x);
      this.steering.y = Number(this.desired.y - this.vel.y);
      // limit to max speed
      this.steering.x =
          Math.min(Math.max(parseInt(this.steering.x), -0.06), 0.06);
      this.steering.y =
          Math.min(Math.max(parseInt(this.steering.y), -0.06), 0.06);
      return this.steering;
    } else {
      return {'x': 0, 'y': 0};
    };
  };

  dist(target, pos) {
    return Math.sqrt(
        Math.pow(target.x - pos.x, 2) + Math.pow(target.y - pos.y, 2));
  };

  update() {
    this.pos.x += this.vel.x;
    this.pos.y += this.vel.y;
    this.vel.x += this.acc.x;
    this.vel.y += this.acc.y;
    this.acc.x = 0;
    this.acc.y = 0;
  };

  show() {
    this.ctx.beginPath();
    this.ctx.arc(this.pos.x, this.pos.y, this.r, 0, Math.PI * 2, false);
    this.ctx.fill();
    this.ctx.closePath();
  };
};

let mouse = {'x': 0, 'y': 0};

Number.prototype.map = function(in_min, in_max, out_min, out_max) {
  return (this - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

// Initialize the page.
window.onload = function() {
  let synthPad = new SynthPad();
};
