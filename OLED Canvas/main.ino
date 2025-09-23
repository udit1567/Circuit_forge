/*
  ESP32 OLED Studio (Light Theme, Image Upload + Dithering)
  - Web UI hosted from PROGMEM (UTF-8)
  - WebSocket binary frames (1024 bytes) -> SSD1306 128x64
  - Requires: ESPAsyncWebServer, AsyncTCP, Adafruit_GFX, Adafruit_SSD1306
*/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Set these:
const char* ssid     = "Excitel_ 2.4";
const char* password = "@Udit1588";

// I2C pins (default SDA=21, SCL=22 on most ESP32 boards)
#ifndef OLED_ADDR
#define OLED_ADDR 0x3C
#endif

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

uint8_t frameBuf[SCREEN_WIDTH * SCREEN_HEIGHT / 8]; // 1024 bytes

const char index_html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>ESP32 OLED Studio</title>
<style>
  :root {
    --bg:#f6f8fb; --panel:#ffffff; --accent:#1565c0; --text:#0b1220;
    --muted:#58677a; --border:#dbe3ee; --btn:#0f1726; --btn-bg:#eef3fb;
  }
  * { box-sizing:border-box; }
  body { margin:0; background:var(--bg); color:var(--text); font-family:Inter,system-ui,Arial,sans-serif; }
  header { padding:16px 20px; border-bottom:1px solid var(--border); display:flex; align-items:center; gap:12px; background:#fff; }
  header h1 { font-size:18px; margin:0; font-weight:600; letter-spacing:0.2px; }
  header .status { margin-left:auto; font-size:13px; color:var(--muted); }
  .container { display:grid; grid-template-columns: 1fr 360px; gap:16px; padding:16px; }
  .panel { background:var(--panel); border:1px solid var(--border); border-radius:12px; overflow:hidden; box-shadow:0 1px 2px rgba(16,24,40,0.03); }
  .panel h3 { margin:0; padding:12px 14px; font-size:14px; border-bottom:1px solid var(--border); color:var(--muted); background:#fafcff; }
  .canvas-wrap { display:flex; align-items:center; justify-content:center; padding:16px; }
  #canvas { width:512px; height:256px; image-rendering: pixelated; background:#000; border-radius:10px; border:1px solid var(--border); }
  .right { display:flex; flex-direction:column; gap:16px; }
  .row { display:flex; gap:8px; flex-wrap:wrap; }
  .ctrl { padding:12px 14px; display:flex; flex-direction:column; gap:10px; }
  label { font-size:12px; color:var(--muted); }
  input[type="range"]{ width:100%; }
  button, select, input[type="text"], input[type="url"] {
    background:#fff; border:1px solid var(--border); color:var(--text);
    padding:10px 12px; border-radius:8px; font-size:14px;
  }
  button.primary { background:#2563eb; color:#fff; border-color:#1d4ed8; }
  button.bad { background:#fee2e2; border-color:#fecaca; color:#7f1d1d; }
  .toolbar button { flex:1; }
  .drop { padding:14px; border:1px dashed var(--border); border-radius:10px; text-align:center; color:var(--muted); background:#fafcff; }
  .grid { position:relative; }
  .grid::after{
    content:""; position:absolute; inset:16px; pointer-events:none;
    background:
      linear-gradient(to right, rgba(0,0,0,0.06) 1px, transparent 1px) 0 0/4px 4px,
      linear-gradient(to bottom, rgba(0,0,0,0.06) 1px, transparent 1px) 0 0/4px 4px;
    display:none;
  }
  .grid.show::after{ display:block; }
  .hint { font-size:12px; color:var(--muted); }
</style>
</head>
<body>
<header>
  <h1>ESP32 OLED Studio</h1>
  <div class="status">WS: <span id="wsstat">connecting…</span></div>
</header>

<div class="container">
  <div class="panel grid" id="gridPanel">
    <h3>Canvas 128×64</h3>
    <div class="canvas-wrap">
      <canvas id="canvas" width="128" height="64"></canvas>
    </div>
  </div>

  <div class="right">
    <div class="panel">
      <h3>Input</h3>
      <div class="ctrl">
        <div class="row">
          <input id="file" type="file" accept="image/*">
          <input id="url" type="url" placeholder="Image URL (CORS required)">
          <button id="loadUrl">Load URL</button>
        </div>
        <div id="drop" class="drop">Drop image here</div>
        <div class="row">
          <select id="fit">
            <option value="fit" selected>Fit (letterbox)</option>
            <option value="fill">Fill (crop)</option>
          </select>
          <button id="invert">Invert</button>
          <button id="rot">Rotate 180°</button>
          <button id="flipX">Flip X</button>
          <button id="flipY">Flip Y</button>
        </div>
        <div class="row toolbar">
          <button id="mode">Draw</button>
          <button id="erase">Erase</button>
          <button id="clear" class="bad">Clear</button>
        </div>
        <div>
          <label>Line width: <span id="lwval">2</span></label>
          <input id="lw" type="range" min="1" max="6" value="2">
        </div>
        <div class="row">
          <input id="text" type="text" placeholder="Add text…">
          <select id="font">
            <option>10px</option><option selected>14px</option><option>18px</option><option>24px</option>
          </select>
          <button id="put">Put Text</button>
        </div>
      </div>
    </div>

    <div class="panel">
      <h3>1‑bit processing</h3>
      <div class="ctrl">
        <div class="row">
          <select id="dither">
            <option value="none">Threshold</option>
            <option value="bayer">Ordered (Bayer)</option>
            <option value="fs">Floyd–Steinberg</option>
          </select>
          <label>Threshold: <span id="tval">128</span></label>
        </div>
        <input id="thresh" type="range" min="0" max="255" value="128">
        <div class="row">
          <label><input id="live" type="checkbox"> Live 1‑bit preview</label>
          <label><input id="grid" type="checkbox"> Pixel grid</label>
          <label><input id="autosend" type="checkbox" checked> Auto‑send</label>
        </div>
        <div class="row">
          <button id="send" class="primary">Send to OLED</button>
          <span class="hint">Sends 1024‑byte binary frame</span>
        </div>
      </div>
    </div>
  </div>
</div>

<script>
const W=128, H=64;
const c = document.getElementById('canvas');
const ctx = c.getContext('2d', { willReadFrequently:true });
const gridPanel = document.getElementById('gridPanel');

let drawing=false, erasing=false, lastX=0, lastY=0;
ctx.fillStyle='#000'; ctx.fillRect(0,0,W,H);
ctx.strokeStyle='#fff'; ctx.lineWidth=2; ctx.lineCap='round';

// Keep last decoded image to reapply Fit/Fill reliably
let srcBmp = null;

// Offscreen original for previewing 1-bit safely
const off = document.createElement('canvas'); off.width=W; off.height=H;
const offCtx = off.getContext('2d');

function setGrid(show){ gridPanel.classList.toggle('show', show); }

function fitDraw(imgBitmap, mode){
  const iw = imgBitmap.width, ih = imgBitmap.height;
  const r = Math.min(W/iw, H/ih);
  const R = Math.max(W/iw, H/ih);
  ctx.save();
  ctx.setTransform(1,0,0,1,0,0);
  ctx.fillStyle='#000'; ctx.fillRect(0,0,W,H);
  let tw, th, tx, ty;
  if (mode==='fill'){ tw = iw*R; th = ih*R; tx=(W-tw)/2; ty=(H-th)/2; }
  else { tw = iw*r; th = ih*r; tx=(W-tw)/2; ty=(H-th)/2; }
  ctx.imageSmoothingEnabled = true;
  ctx.drawImage(imgBitmap, tx, ty, tw, th);
  ctx.restore();
  offCtx.drawImage(c,0,0);
  maybeAutoSend();
}

async function loadBlob(blob){
  if (srcBmp && srcBmp.close) try{ srcBmp.close(); }catch(e){}
  srcBmp = await createImageBitmap(blob);
  fitDraw(srcBmp, document.getElementById('fit').value);
}

document.getElementById('file').addEventListener('change', async e=>{
  const f = e.target.files[0]; if(!f) return;
  await loadBlob(f);
});

document.getElementById('loadUrl').addEventListener('click', async ()=>{
  const u = document.getElementById('url').value;
  if(!u) return;
  const res = await fetch(u, {mode:'cors'});
  const blob = await res.blob();
  await loadBlob(blob);
});

const drop = document.getElementById('drop');
drop.addEventListener('dragover', e=>{ e.preventDefault(); drop.style.background='#f0f4ff'; });
drop.addEventListener('dragleave', ()=>{ drop.style.background=''; });
drop.addEventListener('drop', async e=>{
  e.preventDefault(); drop.style.background='';
  const f = e.dataTransfer.files[0]; if(f) await loadBlob(f);
});

// Draw & erase
function pos(e){
  const r = c.getBoundingClientRect();
  const x = Math.floor((e.clientX - r.left) * (W / r.width));
  const y = Math.floor((e.clientY - r.top)  * (H / r.height));
  return {x: Math.max(0,Math.min(W-1,x)), y:Math.max(0,Math.min(H-1,y))};
}
c.addEventListener('mousedown', e=>{ drawing=true; const p=pos(e); lastX=p.x; lastY=p.y; });
c.addEventListener('mousemove', e=>{
  if(!drawing) return;
  const p=pos(e);
  ctx.strokeStyle = erasing ? '#000' : '#fff';
  ctx.beginPath(); ctx.moveTo(lastX,lastY); ctx.lineTo(p.x,p.y); ctx.stroke();
  lastX=p.x; lastY=p.y;
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});
window.addEventListener('mouseup', ()=>{ drawing=false; offCtx.drawImage(c,0,0); });

c.addEventListener('touchstart', e=>{ const t=e.touches[0]; const p=pos(t); drawing=true; lastX=p.x; lastY=p.y; e.preventDefault(); }, {passive:false});
c.addEventListener('touchmove', e=>{ if(!drawing) return; const t=e.touches[0]; const p=pos(t);
  ctx.strokeStyle = erasing ? '#000' : '#fff';
  ctx.beginPath(); ctx.moveTo(lastX,lastY); ctx.lineTo(p.x,p.y); ctx.stroke();
  lastX=p.x; lastY=p.y; if(document.getElementById('live').checked) preview1bpp(); maybeAutoSend(); e.preventDefault(); }, {passive:false});
c.addEventListener('touchend', ()=>{ drawing=false; offCtx.drawImage(c,0,0); });

document.getElementById('lw').addEventListener('input', e=>{
  ctx.lineWidth = Number(e.target.value);
  document.getElementById('lwval').textContent = e.target.value;
});

document.getElementById('mode').addEventListener('click', ()=>{ erasing=false; });
document.getElementById('erase').addEventListener('click', ()=>{ erasing=true; });

document.getElementById('clear').addEventListener('click', ()=>{
  ctx.fillStyle='#000'; ctx.fillRect(0,0,W,H);
  offCtx.drawImage(c,0,0);
  maybeAutoSend();
});

document.getElementById('put').addEventListener('click', ()=>{
  const txt = document.getElementById('text').value || '';
  const size = document.getElementById('font').value;
  ctx.fillStyle='#000'; ctx.fillRect(0,0,W,H);
  ctx.fillStyle='#fff'; ctx.font = `bold ${size} monospace`; ctx.textBaseline='top';
  ctx.fillText(txt, 0, 0);
  offCtx.drawImage(c,0,0);
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});

document.getElementById('invert').addEventListener('click', ()=>{
  const img = ctx.getImageData(0,0,W,H), d=img.data;
  for(let i=0;i<d.length;i+=4){ d[i]=255-d[i]; d[i+1]=255-d[i+1]; d[i+2]=255-d[i+2]; }
  ctx.putImageData(img,0,0);
  offCtx.drawImage(c,0,0);
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});
document.getElementById('rot').addEventListener('click', ()=>{
  ctx.save(); ctx.translate(W/2,H/2); ctx.rotate(Math.PI); ctx.drawImage(off,-W/2,-H/2); ctx.restore();
  offCtx.drawImage(c,0,0);
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});
document.getElementById('flipX').addEventListener('click', ()=>{
  ctx.save(); ctx.translate(W,0); ctx.scale(-1,1); ctx.drawImage(off,0,0); ctx.restore();
  offCtx.drawImage(c,0,0);
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});
document.getElementById('flipY').addEventListener('click', ()=>{
  ctx.save(); ctx.translate(0,H); ctx.scale(1,-1); ctx.drawImage(off,0,0); ctx.restore();
  offCtx.drawImage(c,0,0);
  if(document.getElementById('live').checked) preview1bpp();
  maybeAutoSend();
});

// Re-apply scaling when Fit/Fill changes
document.getElementById('fit').addEventListener('change', ()=>{
  if (srcBmp) fitDraw(srcBmp, document.getElementById('fit').value);
});

// Threshold & dither
const thresh = document.getElementById('thresh');
thresh.addEventListener('input', ()=>{
  document.getElementById('tval').textContent = thresh.value;
  if(document.getElementById('live').checked) preview1bpp();
});
document.getElementById('dither').addEventListener('change', ()=>{
  if(document.getElementById('live').checked) preview1bpp();
});
document.getElementById('live').addEventListener('change', ()=>{
  if(document.getElementById('live').checked) preview1bpp(); else ctx.drawImage(off,0,0);
});
document.getElementById('grid').addEventListener('change', e=> setGrid(e.target.checked));

function toGray(imgData){
  const d = imgData.data, g = new Float32Array(W*H);
  for(let i=0, j=0;i<d.length;i+=4, j++){ g[j] = 0.2126*d[i] + 0.7152*d[i+1] + 0.0722*d[i+2]; }
  return g;
}
function preview1bpp(){
  const id = offCtx.getImageData(0,0,W,H);
  const mode = document.getElementById('dither').value;
  const t = Number(thresh.value);
  if(mode==='none'){
    for(let i=0;i<id.data.length;i+=4){
      const v = (0.2126*id.data[i] + 0.7152*id.data[i+1] + 0.0722*id.data[i+2]) > t ? 255 : 0;
      id.data[i]=id.data[i+1]=id.data[i+2]=v; id.data[i+3]=255;
    }
  } else if(mode==='bayer'){
    const b4 = [
      [  0,128, 32,160],
      [192, 64,224, 96],
      [ 48,176, 16,144],
      [240,112,208, 80]
    ];
    for(let y=0;y<H;y++){
      for(let x=0;x<W;x++){
        const i=(y*W+x)*4;
        const v = 0.2126*id.data[i] + 0.7152*id.data[i+1] + 0.0722*id.data[i+2];
        const threshLocal = (b4[y&3][x&3]/255)*255*0.75 + t*0.25;
        const out = v > threshLocal ? 255 : 0;
        id.data[i]=id.data[i+1]=id.data[i+2]=out; id.data[i+3]=255;
      }
    }
  } else {
    const g = toGray(id);
    for(let y=0;y<H;y++){
      for(let x=0;x<W;x++){
        const idx = y*W+x;
        const old = g[idx];
        const newv = old > t ? 255 : 0;
        const err = old - newv;
        g[idx] = newv;
        if(x+1<W) g[idx+1] += err*7/16;
        if(y+1<H){
          if(x>0) g[idx+W-1] += err*3/16;
          g[idx+W] += err*5/16;
          if(x+1<W) g[idx+W+1] += err*1/16;
        }
      }
    }
    for(let i=0, j=0;i<id.data.length;i+=4, j++){
      const v = g[j] >= 128 ? 255 : 0;
      id.data[i]=id.data[i+1]=id.data[i+2]=v; id.data[i+3]=255;
    }
  }
  ctx.putImageData(id,0,0);
}

// Pack and send
function packTo1bpp(){
  const img = ctx.getImageData(0,0,W,H).data;
  const buf = new Uint8Array((W*H)>>3);
  for(let y=0;y<H;y++){
    for(let x=0;x<W;x++){
      const i=(y*W+x)*4;
      const on = img[i] > 127;
      if(on){
        const bi = y*(W>>3) + (x>>3);
        buf[bi] |= (1 << (7-(x&7)));
      }
    }
  }
  return buf;
}

let ws;
function connectWS(){
  const url = `ws://${location.host}/ws`;
  ws = new WebSocket(url);
  ws.binaryType='arraybuffer';
  ws.onopen = ()=> document.getElementById('wsstat').textContent='connected';
  ws.onclose = ()=>{ document.getElementById('wsstat').textContent='reconnecting…'; setTimeout(connectWS,1200); };
  ws.onerror = ()=> ws.close();
}
connectWS();

let sendTimer=null;
function maybeAutoSend(){
  if(!document.getElementById('autosend').checked) return;
  clearTimeout(sendTimer);
  sendTimer = setTimeout(()=> doSend(), 200);
}
function doSend(){
  if(!ws || ws.readyState!==1) return;
  if(!document.getElementById('live').checked) preview1bpp();
  const buf = packTo1bpp();
  ws.send(buf.buffer);
}

document.getElementById('send').addEventListener('click', doSend);
document.getElementById('thresh').addEventListener('change', maybeAutoSend);
document.getElementById('dither').addEventListener('change', maybeAutoSend);
document.getElementById('fit').addEventListener('change', ()=> maybeAutoSend());
</script>
</body>
</html>
)HTML";

// WebSocket event handler
// Replace your onWsEvent with this version
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  static size_t rx_total = 0;
  static bool rx_accept = false;

  if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;

    if (info->opcode == WS_BINARY) {
      if (info->index == 0) {
        // New binary message started
        rx_total = 0;
        rx_accept = (info->len <= sizeof(frameBuf)); // expect 1024, but accept <= buffer
        if (!rx_accept) {
          Serial.printf("WS BIN too big: %u bytes\n", (unsigned)info->len);
        }
      }

      // Copy this fragment into the correct position
      if (rx_accept) {
        size_t toCopy = len;
        if (info->index + toCopy > sizeof(frameBuf)) {
          toCopy = sizeof(frameBuf) - info->index; // clamp
        }
        memcpy(frameBuf + info->index, data, toCopy);
        rx_total = info->index + toCopy;
      }

      // If this is the last fragment, render
      if (info->final) {
        Serial.printf("WS BIN final idx=%u chunk=%u total=%u expected=%u\n",
                      (unsigned)info->index, (unsigned)len,
                      (unsigned)rx_total, (unsigned)info->len);
        if (rx_accept && info->len == (SCREEN_WIDTH*SCREEN_HEIGHT/8)) {
          display.clearDisplay();
          display.drawBitmap(0, 0, frameBuf, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
          display.display();
          if (client) client->text("ok"); // ACK to browser
        } else {
          if (client) client->text("badlen"); // helpful error
        }
      }
    } else if (info->opcode == WS_TEXT) {
      if (len >= 5 && !strncmp((char*)data, "clear", 5)) {
        display.clearDisplay();
        display.display();
        if (client) client->text("ok");
      }
    }

  } else if (type == WS_EVT_CONNECT) {
    Serial.printf("WS client #%u connected from %s\n",
                  client->id(), client->remoteIP().toString().c_str());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WS client #%u disconnected\n", client->id());
  }
}


void setup() {
  Serial.begin(115200);
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed");
    while (true) { delay(1000); }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("ESP32 OLED Studio");
  display.display();

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.print("\nIP: "); Serial.println(WiFi.localIP());

  // Web server + WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Serve page (explicit UTF-8 to avoid mojibake)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html; charset=utf-8", index_html);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
}
