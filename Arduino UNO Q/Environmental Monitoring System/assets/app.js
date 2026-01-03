const canvas = document.getElementById('plot');
const ctx = canvas.getContext('2d');
const classesRoot = document.getElementById('classes');

const maxSamples = 200;
const samples = [];

function drawPlot() {
  ctx.clearRect(0,0,canvas.width,canvas.height);

  function drawSeries(key, color) {
    ctx.strokeStyle = color;
    ctx.beginPath();
    samples.forEach((s,i)=>{
      const x = (i/(maxSamples-1))*canvas.width;
      const y = canvas.height/2 - s[key]*2;
      if(i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
    });
    ctx.stroke();
  }

  drawSeries('co2','#0068C9');
  drawSeries('co','#FF9900');
  drawSeries('lpg','#FF2B2B');
}

function pushSample(s){
  samples.push(s);
  if(samples.length>maxSamples) samples.shift();
  drawPlot();
}

function renderGas(data) {
  classesRoot.innerHTML = '';

  Object.entries(data).forEach(([key, value]) => {
    const card = document.createElement('div');
    card.className = 'data-card';

    card.innerHTML = `
      <div class="label">${key}</div>
      <div class="value">
        ${value.toFixed(2)}
        <span class="unit">${getUnit(key)}</span>
      </div>
    `;

    classesRoot.appendChild(card);
  });
}

function getUnit(key) {
  if (key.includes('temp')) return '°C';
  if (key.includes('pressure')) return 'hPa';
  if (key.includes('co') || key.includes('lpg') || key.includes('smoke')) return 'ppm';
  return '';
}

fetch('/samples').then(r=>r.json()).then(list=>list.forEach(pushSample));
fetch('/gas').then(r=>r.json()).then(renderGas);

const socket = io();
socket.on('sample', pushSample);
socket.on('gas', renderGas);
