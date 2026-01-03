/*
 * SPDX-FileCopyrightText: Copyright (C) ARDUINO SRL (http://www.arduino.cc)
 *
 * SPDX-License-Identifier: MPL-2.0
 */

const classesRoot = document.getElementById('classes');

// plotting helpers
const canvas = document.getElementById('plot');
const ctx = canvas.getContext('2d');
const width = canvas.width, height = canvas.height;
const maxSamples = 200;
const samples = [];
let errorContainer;

function drawPlot() {
  // clear
  ctx.fillStyle = '#fff';
  ctx.fillRect(0,0,width,height);

  // All grid lines (every 0.5) - same size
  ctx.strokeStyle = '#f0f0f0';
  ctx.lineWidth = 0.5;
  ctx.beginPath();
  for (let i=0; i<=8; i++){
    const y = 10 + i*((height-20)/8);
    ctx.moveTo(40,y);
    ctx.lineTo(width,y);
  }
  ctx.stroke();

  // Y-axis labels (-2.0 to 2.0 every 0.5)
  ctx.fillStyle = '#666';
  ctx.font = '11px Arial';
  ctx.textAlign = 'right';
  ctx.textBaseline = 'middle';

  for (let i=0; i<=8; i++) {
    const y = 10 + i*((height-20)/8);
    const value = (2.0 - i * 0.5).toFixed(1);
    ctx.fillText(value, 35, y);
  }

  // draw each series
  function drawSeries(key, color) {
    ctx.strokeStyle = color;
    ctx.lineWidth = 1;
    ctx.beginPath();
    for (let i=0;i<samples.length;i++){
      const s = samples[i];
      const x = 40 + (i/(maxSamples-1))*(width-40);
      const v = s[key];
      const y = (height/2) - (v * ((height-20)/4));
      if (i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
    }
    ctx.stroke();
  }

  drawSeries('x','#0068C9');
  drawSeries('y','#FF9900');
  drawSeries('z','#FF2B2B');
}

function pushSample(s){
  samples.push(s);
  if (samples.length>maxSamples) samples.shift();
  drawPlot();
}

function renderClasses(d){
  const orderedKeys = ['snake', 'wave', 'updown', 'idle'];
  
  let maxKey = null;
  let maxValue = -1;
  for (const key in d) {
    if (d[key] > maxValue) {
      maxValue = d[key];
      maxKey = key;
    }
  }

  classesRoot.innerHTML = '';

  const icons = {
    snake: '🐍',
    updown: '↕',
    wave: '🌊',
    idle: '💤'
  };

  const names = {
    snake: 'Snake',
    updown: 'Up Down',
    wave: 'Wave',
    idle: 'Idle'
  };

  orderedKeys.forEach(key => {
    const value = d[key] || 0;
    const row = document.createElement('div');
    row.className = 'row';
    const icon = icons[key] || key;
    const name = names[key] || key;
    const percentage = value.toFixed(1);

    const isMax = key === maxKey;
    const progressClass = isMax ? 'progress-fill primary' : 'progress-fill secondary';
    const valueClass = isMax ? 'value primary' : 'value secondary';

    row.innerHTML = `
      <div class="movement-header">
        <div class="label"><span class="emoji">${icon}</span> ${name}</div>
        <div class="${valueClass}">${percentage}%</div>
      </div>
      <div class="progress-bar">
        <div class="${progressClass}" style="width: ${percentage}%"></div>
      </div>
    `;
    classesRoot.appendChild(row);
  });
}

function setValues(d) {
  renderClasses(d);
}

// Try to fetch current value via HTTP first
fetch('/detection').then(r => r.json()).then(d => {
  console.debug('Fetched /detection', d);
  setValues(d);
}).catch((e) => {
  console.debug('Failed to fetch /detection', e);
});

// Fetch recent samples on load
fetch('/samples').then(r=>r.json()).then(list=>{
  if (Array.isArray(list)){
    list.forEach(s => pushSample(s));
  }
}).catch(e=>console.debug('Failed to load /samples',e));

// Connect explicitly using the full origin and the /socket.io path
const serverOrigin = window.location.origin;
console.debug('Attempting socket.io connect to', serverOrigin);
const socket = io(serverOrigin, {
  path: '/socket.io',
  transports: ['polling','websocket'],
  autoConnect: true
});

socket.on('movement', (data) => {
  console.debug('received movement', data);
  setValues(data);
});

socket.on('sample', (s) => {
  pushSample(s);
});

socket.on('connect', () => {
  if (errorContainer) {
    errorContainer.style.display = 'none';
    errorContainer.textContent = '';
  }
});

socket.on('disconnect', () => {
  errorContainer = document.getElementById('error-container');
  if (errorContainer) {
    errorContainer.textContent = 'Connection to the board lost. Please check the connection.';
    errorContainer.style.display = 'block';
  }
});